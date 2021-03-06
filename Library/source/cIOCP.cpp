
#include "stdafx.h"
#include "LibraryDef.h"
#include "cIOCP.h"
#include "cLog.h"


static unsigned int WINAPI CallWorkerThread( LPVOID p )
{
	cIOCP* pOverlappedEvent = static_cast<cIOCP*>( p );
	pOverlappedEvent->WorkerThread();
	return 0;
}

static unsigned int WINAPI CallAccepterThread( LPVOID p )
{
	cIOCP* pOverlappedEvent = static_cast<cIOCP*>( p );
	pOverlappedEvent->AccepterThread();
	return 0;
}


cIOCP::cIOCP()
	: m_bWorkerRun { true }, m_bAccepterRun { true }
	, m_nClientCnt { 0 }
	, m_hAccepterThread { NULL }
	, m_hIOCP { NULL }
	, m_socketListen { INVALID_SOCKET }
	, m_vWorkerThread( MAX_WORKERTHREAD, NULL )
	, m_vClientInfo( MAX_CLIENT )
{
	ZeroMemory( m_szSocketBuf, 1024 );
}

cIOCP::~cIOCP()
{
	WSACleanup();
}

bool cIOCP::InitSocket()
{
	WSADATA wsaData;
	int nRet = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if ( 0 != nRet )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::InitSocket() | WSAStartup() Failed : ( " ) } + wsp::to( WSAGetLastError() ) + _T( " )" ) );
		return false;
	}

	m_socketListen = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED );
	if ( INVALID_SOCKET == m_socketListen )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::InitSocket() | WSASocket() Failed : ( " ) } + wsp::to( WSAGetLastError() ) + _T( " )" ) );
		return false;
	}

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | cIOCP::InitSocket() | 소켓 초기화 성공" ) );

	return true;
}

void cIOCP::CloseSocket( stClientInfo& clientInfo, const bool bIsForce )
{
	struct linger stLinger { 0, 0 };

	if ( true == bIsForce )
		stLinger.l_onoff = 1;

	shutdown( clientInfo.m_socketClient, SD_BOTH );

	setsockopt( clientInfo.m_socketClient, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>( &stLinger ), sizeof( stLinger ) );

	closesocket( clientInfo.m_socketClient );

	clientInfo.m_socketClient = INVALID_SOCKET;
}

bool cIOCP::BindandListen( int nBindPort )
{
	SOCKADDR_IN		stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons( nBindPort );
	stServerAddr.sin_addr.s_addr = htonl( INADDR_ANY );

	int nRet = bind( m_socketListen, reinterpret_cast<SOCKADDR*>( &stServerAddr ), sizeof( SOCKADDR_IN ) );
	if ( 0 != nRet )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::BindandListen() | bind() Failed : ( " ) } + wsp::to( WSAGetLastError() ) + _T( " )" ) );
		return false;
	}

	nRet = listen( m_socketListen, 5 );
	if ( 0 != nRet )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::BindandListen() | listen() Failed : ( " ) } + wsp::to( WSAGetLastError() ) + _T( " )" ) );
		return false;
	}

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | cIOCP::BindandListen() | 서버 등록 성공" ) );

	return true;
}

bool cIOCP::CreateWorkerThread()
{
	unsigned int uiThreadId = 0;

	for ( auto hThread : m_vWorkerThread )
	{
		hThread = reinterpret_cast<HANDLE>( _beginthreadex( nullptr, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &uiThreadId ) );
		if ( NULL == hThread )
		{
			LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::CreateWorkerThread() | _beginthreadex() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
			return false;
		}
		ResumeThread( hThread );
	}

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | cIOCP::CreateWorkerThread() | Worker Thread 시작 ..." ) );

	return true;
}

bool cIOCP::CreateAccepterThread()
{
	unsigned int uiThreadId = 0;

	m_hAccepterThread = reinterpret_cast<HANDLE>( _beginthreadex( nullptr, 0, &CallAccepterThread, this, CREATE_SUSPENDED, &uiThreadId ) );
	if ( NULL == m_hAccepterThread )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::CreateAccepterThread() | _beginthreadex() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		return false;
	}
	ResumeThread( m_hAccepterThread );

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | cIOCP::CreateAccepterThread() | Accepter Thread 시작 ..." ) );

	return true;
}

bool cIOCP::BindIOCompletionPort( const stClientInfo& clientInfo )
{
	HANDLE hIOCP;
	hIOCP = CreateIoCompletionPort( reinterpret_cast<HANDLE>( clientInfo.m_socketClient ), m_hIOCP, reinterpret_cast<ULONG_PTR>( &clientInfo ), 0 );
	if ( NULL == hIOCP || m_hIOCP != hIOCP )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::BindIOCompletionPort() | CreateIoCompletionPort() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		return false;
	}

	return true;
}

bool cIOCP::StartServer()
{
	m_hIOCP = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, NULL, 0 );
	if ( NULL == m_hIOCP )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::StartServer() | CreateIoCompletionPort() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		return false;
	}

	bool bRet = CreateWorkerThread();
	if ( false == bRet )
		return false;

	bRet = CreateAccepterThread();
	if ( false == bRet )
		return false;

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | cIOCP::StartServer() | 서버 시작 ..." ) );

	return true;
}

bool cIOCP::BindRecv( stClientInfo& clientInfo )
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	clientInfo.m_stRecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
	clientInfo.m_stRecvOverlappedEx.m_wsaBuf.buf = clientInfo.m_stRecvOverlappedEx.m_szBuf;
	clientInfo.m_stRecvOverlappedEx.m_eOperation = eOperation::OP_RECV;

	int nRet = WSARecv( clientInfo.m_socketClient, &(clientInfo.m_stRecvOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes, &dwFlag, reinterpret_cast<LPWSAOVERLAPPED>( &clientInfo.m_stRecvOverlappedEx ), NULL );
	if ( nRet == SOCKET_ERROR && ( WSAGetLastError() != ERROR_IO_PENDING ) )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::BindRecv() | WSARecv() Failed : ( " ) } + wsp::to( WSAGetLastError() ) + _T( " )" ) );
		return false;
	}

	return true;
}

bool cIOCP::SendMsg( stClientInfo& clientInfo, char* pMsg, int nLen )
{
	DWORD dwRecvNumBytes = 0;

	CopyMemory( clientInfo.m_stSendOverlappedEx.m_szBuf, pMsg, nLen );

	clientInfo.m_stSendOverlappedEx.m_wsaBuf.len = nLen;
	clientInfo.m_stSendOverlappedEx.m_wsaBuf.buf = clientInfo.m_stSendOverlappedEx.m_szBuf;
	clientInfo.m_stSendOverlappedEx.m_eOperation = eOperation::OP_SEND;

	int nRet = WSASend( clientInfo.m_socketClient, &(clientInfo.m_stSendOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes, 0, reinterpret_cast<LPWSAOVERLAPPED>( &clientInfo.m_stSendOverlappedEx ), NULL );
	if ( nRet == SOCKET_ERROR && ( WSAGetLastError() != ERROR_IO_PENDING ) )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::SendMsg() | WSASend() Failed : ( " ) } + wsp::to( WSAGetLastError() ) + _T( " )" ) );
		return false;
	}

	return true;
}

stClientInfo* cIOCP::GetEmptyClientInfo()
{
	for ( auto& clientInfo : m_vClientInfo )
	{
		if ( INVALID_SOCKET == clientInfo.m_socketClient )
			return &clientInfo;
	}

	return nullptr;
}

void cIOCP::AccepterThread()
{
	SOCKADDR_IN		stClientAddr;
	int nAddrLen = sizeof( SOCKADDR_IN );
	while ( m_bAccepterRun )
	{
		stClientInfo* pClientInfo = GetEmptyClientInfo();
		if ( nullptr == pClientInfo )
		{
			LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIOCP::AccepterThread() | Client Full" ) } );
			return;
		}

		pClientInfo->m_socketClient = accept( m_socketListen, (SOCKADDR*)&stClientAddr, &nAddrLen );
		if ( INVALID_SOCKET == pClientInfo->m_socketClient )
			continue;

		bool bRet = BindIOCompletionPort( *pClientInfo );
		if ( false == bRet )
			return;

		bRet = BindRecv( *pClientInfo );
		if ( false == bRet )
			return;

		LOG( eLogInfoType::LOG_INFO_LOW, tstring{ _T( "SYSTEM | cIOCP::AccepterThread() | 클라이언트 접속 : IP( " ) } + Inet_Ntop( stClientAddr ) + _T( " ) SOCKET( " ) + wsp::to( pClientInfo->m_socketClient ) + _T( " )" ) );
		++m_nClientCnt;
	}
}


void cIOCP::WorkerThread()
{
	stClientInfo* pClientInfo {};
	BOOL bSuccess = TRUE;
	DWORD dwIoSize = 0;
	LPOVERLAPPED lpOverlapped = nullptr;

	while ( m_bWorkerRun )
	{
		bSuccess = GetQueuedCompletionStatus( m_hIOCP, &dwIoSize, reinterpret_cast<PULONG_PTR>( &pClientInfo ), &lpOverlapped, INFINITE );
		if ( FALSE == bSuccess && 0 == dwIoSize )
		{
			LOG( eLogInfoType::LOG_INFO_LOW, tstring{ _T( "SYSTEM | cIOCP::WorkerThread() | socket( " ) } + wsp::to( pClientInfo->m_socketClient ) + _T( " ) 접속 끊김" ) );
			CloseSocket( *pClientInfo );
			continue;
		}

		if ( TRUE == bSuccess && 0 == dwIoSize && nullptr == lpOverlapped )
		{
			m_bWorkerRun = false;
			continue;
		}
		if ( nullptr == lpOverlapped )
			continue;

		stOverlappedEx* pOverlappedEx = reinterpret_cast<stOverlappedEx*>( lpOverlapped );
		if ( eOperation::OP_RECV == pOverlappedEx->m_eOperation )
		{
			pOverlappedEx->m_szBuf[ dwIoSize ] = '\0';
			LOG( eLogInfoType::LOG_INFO_LOW, tstring{ _T( "SYSTEM | cIOCP::WorkerThread() | [수신] bytes : " ) } + wsp::to( dwIoSize ) + _T( ", msg : " ) + wsp::to( pOverlappedEx->m_szBuf ) );

			BindRecv( *pClientInfo );
			SendMsg( *pClientInfo, pOverlappedEx->m_wsaBuf.buf, pOverlappedEx->m_wsaBuf.len );
		}
		else if ( eOperation::OP_SEND == pOverlappedEx->m_eOperation )
		{
			LOG( eLogInfoType::LOG_INFO_LOW, tstring{ _T( "SYSTEM | cIOCP::WorkerThread() | [송신] bytes : " ) } + wsp::to( dwIoSize ) + _T( ", msg : " ) + wsp::to( pOverlappedEx->m_szBuf ) );
		}
		else
		{
			LOG( eLogInfoType::LOG_INFO_LOW, tstring{ _T( "SYSTEM | cIOCP::WorkerThread() | socket( " ) } + wsp::to( pClientInfo->m_socketClient ) + _T( " )에서 예외상황" ) );
		}
	}
}

void cIOCP::DestroyThread()
{
	for ( int i = 0 ; i < MAX_WORKERTHREAD ; ++i )
	{
		PostQueuedCompletionStatus( m_hIOCP, 0, 0, nullptr );
	}

	for ( auto hThread : m_vWorkerThread )
	{
		CloseHandle( hThread );
		WaitForSingleObject( hThread, INFINITE );
	}

	m_bAccepterRun = false;
	closesocket( m_socketListen );
	WaitForSingleObject( m_hAccepterThread, INFINITE );
}
