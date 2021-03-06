
#include	"stdafx.h"
#include	"cConnection.h"
#include	"LibraryDef.h"
#include	"cLog.h"
#include	"cIocpServer.h"


cConnection::cConnection()
	: m_sockListener { INVALID_SOCKET }
	, m_socket { INVALID_SOCKET }
	, m_lpRecvOverlappedEx { nullptr }
	, m_lpSendOverlappedEx { nullptr }
	, m_nSendBufSize { 0 }
	, m_nRecvBufSize { 0 }
	, m_pIocpServer { nullptr }
{
	InitializeConnection();
}

void cConnection::InitializeConnection()
{
	m_socket				= INVALID_SOCKET;
	m_bIsConnect			= false;
	m_bIsClosed				= FALSE;
	m_bIsSend				= TRUE;
	m_dwSendIoRefCount		= 0;
	m_dwRecvIoRefCount		= 0;
	m_dwAcceptIoRefCount	= 0;

	m_ringRecvBuffer.Initialize();
	m_ringSendBuffer.Initialize();
}

cConnection::~cConnection()
{
	m_sockListener = INVALID_SOCKET;
	m_socket = INVALID_SOCKET;
	if ( nullptr != m_lpRecvOverlappedEx )
	{
		m_lpRecvOverlappedEx->s_lpConnection.reset();
		delete m_lpRecvOverlappedEx;
	}
	if ( nullptr != m_lpSendOverlappedEx )
	{
		m_lpSendOverlappedEx->s_lpConnection.reset();
		delete m_lpSendOverlappedEx;
	}
	m_pIocpServer = nullptr;
}

bool cConnection::CreateConnection( const INITCONFIG& initConfig )
{
	m_nIndex = initConfig.nIndex;
	m_sockListener = initConfig.sockListener;

	m_pIocpServer = initConfig.pIocpServer;

	m_lpRecvOverlappedEx = new OVERLAPPED_EX( shared_from_this() );
	m_lpSendOverlappedEx = new OVERLAPPED_EX( shared_from_this() );
	m_ringRecvBuffer.Create( initConfig.nRecvBufSize * initConfig.nRecvBufCnt );
	m_ringSendBuffer.Create( initConfig.nSendBufSize * initConfig.nSendBufCnt );

	m_nRecvBufSize = initConfig.nRecvBufSize;
	m_nSendBufSize = initConfig.nSendBufSize;

	return BindAcceptExSock();
}

bool cConnection::BindAcceptExSock()
{
	DWORD dwBytes = 0;
	memset( &m_lpRecvOverlappedEx->s_Overlapped, 0, sizeof( OVERLAPPED ) );
	m_lpRecvOverlappedEx->s_WsaBuf.buf = m_szAddressBuf.data();
	m_lpRecvOverlappedEx->s_lpSocketMsg = &m_lpRecvOverlappedEx->s_WsaBuf.buf[ 0 ];
	m_lpRecvOverlappedEx->s_WsaBuf.len = m_nRecvBufSize;
	m_lpRecvOverlappedEx->s_eOperation = eOperationType::OP_ACCEPT;
	m_lpRecvOverlappedEx->s_lpConnection = shared_from_this();
	m_socket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED );
	if ( INVALID_SOCKET == m_socket )
	{
		LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cConnection::BindAcceptExSock() | WSASocket() Failed : error[ " ) } + wsp::to( GetLastError() ) + _T( " ]" ) );
		return false;
	}

	IncrementAcceptIoRefCount();

	BOOL bRet = AcceptEx( m_sockListener, m_socket, m_lpRecvOverlappedEx->s_WsaBuf.buf, 0, sizeof( SOCKADDR_IN ) + 16, sizeof( SOCKADDR_IN ) + 16, &dwBytes, (LPOVERLAPPED)m_lpRecvOverlappedEx );
	if ( !bRet && WSAGetLastError() != WSA_IO_PENDING )
	{
		DecrementAcceptIoRefCount();
		LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cConnection::BindAcceptExSock() | AcceptEx() Failed : error[ " ) } + wsp::to( GetLastError() ) + _T( " ]" ) );
		return false;
	}
	return true;
}

bool cConnection::CloseConnection( bool bForce )
{
	cMonitor::Owner lock { m_csConnection };
	{
		struct linger li = { 0, 0 };	// Default : SO_DONTLINGER
		if ( bForce )
			li.l_onoff = 1;				// SO_LINGER, timeout = 0
		if ( nullptr != IocpServer() && true == m_bIsConnect )
			IocpServer()->OnClose( shared_from_this() );
		shutdown( m_socket, SD_BOTH );
		setsockopt( m_socket, SOL_SOCKET, SO_LINGER, (char*)&li, sizeof( li ) );
		closesocket( m_socket );

		m_socket = INVALID_SOCKET;
		if ( m_lpRecvOverlappedEx != nullptr )
		{
			m_lpRecvOverlappedEx->s_dwRemain = 0;
			m_lpRecvOverlappedEx->s_nTotalBytes = 0;
		}
		if ( m_lpSendOverlappedEx != nullptr )
		{
			m_lpSendOverlappedEx->s_dwRemain = 0;
			m_lpSendOverlappedEx->s_nTotalBytes = 0;
		}
		InitializeConnection();
		BindAcceptExSock();
	}
	return true;
}

char* cConnection::PrepareSendPacket( int slen )
{
	if ( false == m_bIsConnect )
		return nullptr;
	char* pBuf = m_ringSendBuffer.ForwardMark( slen );
	if ( nullptr == pBuf )
	{
		IocpServer()->CloseConnection( shared_from_this() );
		LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cConnection::PrepareSendPacket() | Socket[ " ) } + wsp::to( m_socket ) + _T( " ] SendRingBuffer overflow" ) );
		return nullptr;
	}

	std::fill( pBuf, pBuf + slen, 0 );
	CopyMemory( pBuf, &slen, PACKET_SIZE_LENGTH );

	return pBuf;
}

bool cConnection::ReleaseSendPacket( LPOVERLAPPED_EX lpSendOverlappedEx )
{
	if ( nullptr == lpSendOverlappedEx )
		return false;

	m_ringSendBuffer.ReleaseBuffer( m_lpSendOverlappedEx->s_WsaBuf.len );
	lpSendOverlappedEx = nullptr;

	return true;
}

bool cConnection::BindIOCP( HANDLE& hIOCP )
{
	HANDLE hIOCPHandle;
	cMonitor::Owner lock { m_csConnection };

	hIOCPHandle = CreateIoCompletionPort( (HANDLE)m_socket, hIOCP, reinterpret_cast<ULONG_PTR>( this ), 0 );
	if ( NULL == hIOCPHandle || hIOCP != hIOCPHandle )
	{
		LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cConnection::BindIOCP() | CreateIoCompletionPort() Failed : " ) } + wsp::to( GetLastError() ) );
		return false;
	}

	m_hIOCP = hIOCP;

	return true;
}

bool cConnection::RecvPost( char* pNext, DWORD dwRemain )
{
	int nRet = 0;
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	if ( false == m_bIsConnect || nullptr == m_lpRecvOverlappedEx )
		return false;
	m_lpRecvOverlappedEx->s_eOperation = eOperationType::OP_RECV;
	m_lpRecvOverlappedEx->s_dwRemain = dwRemain;
	int nMoveMark = dwRemain - static_cast<int>( m_ringRecvBuffer.GetCurrentMark() - pNext );
	m_lpRecvOverlappedEx->s_WsaBuf.len = m_nRecvBufSize;
	m_lpRecvOverlappedEx->s_WsaBuf.buf = m_ringRecvBuffer.ForwardMark( nMoveMark, m_nRecvBufSize, dwRemain );
	if ( nullptr == m_lpRecvOverlappedEx->s_WsaBuf.buf )
	{
		IocpServer()->CloseConnection( shared_from_this() );
		LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cConnection::RecvPost() | Socket[ " ) } + wsp::to( m_socket ) + _T( " ] RecvRingBuffer overflow ..." ) );
		return false;
	}

	m_lpRecvOverlappedEx->s_lpSocketMsg = m_lpRecvOverlappedEx->s_WsaBuf.buf - dwRemain;

	memset( &m_lpRecvOverlappedEx->s_Overlapped, 0, sizeof( OVERLAPPED ) );
	IncrementRecvIoRefCount();

	int ret = WSARecv( m_socket, &m_lpRecvOverlappedEx->s_WsaBuf, 1, &dwRecvNumBytes, &dwFlag, &m_lpRecvOverlappedEx->s_Overlapped, NULL );
	if ( ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING )
	{
		DecrementRecvIoRefCount();
		IocpServer()->CloseConnection( shared_from_this() );
		LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cConnection::RecvPost() | WSARecv() Failed : " ) } + wsp::to( GetLastError() ) );
		return false;
	}

	return true;
}

bool cConnection::SendPost()
{
	DWORD dwBytes;
	if ( InterlockedCompareExchange( (LPLONG)&m_bIsSend, FALSE, TRUE ) == TRUE )
	{
		int nReadSize;
		char* pBuf = m_ringSendBuffer.GetBuffer( m_nSendBufSize, &nReadSize );
		if ( nullptr == pBuf )
		{
			InterlockedExchange( (LPLONG)&m_bIsSend, TRUE );
			return false;
		}

		m_lpSendOverlappedEx->s_dwRemain = 0;
		m_lpSendOverlappedEx->s_eOperation = eOperationType::OP_SEND;
		m_lpSendOverlappedEx->s_nTotalBytes = nReadSize;
		ZeroMemory( &m_lpSendOverlappedEx->s_Overlapped, sizeof( OVERLAPPED ) );
		m_lpSendOverlappedEx->s_WsaBuf.len = nReadSize;
		m_lpSendOverlappedEx->s_WsaBuf.buf = pBuf;
		m_lpSendOverlappedEx->s_lpConnection = shared_from_this();

		IncrementSendIoRefCount();

		int ret = WSASend( m_socket, &m_lpSendOverlappedEx->s_WsaBuf, 1, &dwBytes, 0, &m_lpSendOverlappedEx->s_Overlapped, NULL );
		if ( ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING )
		{
			DecrementSendIoRefCount();
			IocpServer()->CloseConnection( shared_from_this() );
			LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "[ERROR] socket[ " ) } + wsp::to( m_socket ) + _T( " ] WSASend() : SOCKET_ERROR, " ) + wsp::to( WSAGetLastError() ) );
		}
		InterlockedExchange( (LPLONG)&m_bIsSend, FALSE );
		return false;
	}
	return true;
}

void cConnection::SetConnectionIp( SOCKADDR_IN* Ip )
{
	if ( Ip )
	{
		m_szIp = Inet_Ntop( *Ip );
	}
}
