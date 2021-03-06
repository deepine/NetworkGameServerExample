
#include	"stdafx.h"
#include	"cIocpServer.h"
#include	"cIocpServer_Impl.h"
#include	"cLog.h"
#include	"cConnection.h"


static unsigned int WINAPI CallWorkerThread( LPVOID p )
{
	cIocpServer* pServer = static_cast<cIocpServer*>( p );
	pServer->WorkerThread();
	return 0;
}

static unsigned int WINAPI CallProcessThread( LPVOID p )
{
	cIocpServer* pServer = static_cast<cIocpServer*>( p );
	pServer->ProcessThread();
	return 0;
}


bool cIocpServer::InitializeSocket()
{
	WSADATA wsaData;
	int nRet = WSAStartup( MAKEWORD( 2, 2 ), & wsaData );
	if ( 0 != nRet )
	{
		LOG( eLogInfoType::LOG_ERROR_HIGH, tstring{ _T( "SYSTEM | cIocpServer::InitializeSocket() | WSAStartup() Failed : ( " ) } + wsp::to( WSAGetLastError() ) + _T( " )" ) );
		return false;
	}

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | sIocpServer::InitializeSocket() | 소켓 초기화 성공" ) );

	return true;
}

bool cIocpServer::ServerStart( INITCONFIG& initConfig )
{
	m_usPort = initConfig.nServerPort;
	m_dwWorkerThreadCount = initConfig.nWokerThreadCnt;
	m_dwProcessThreadCount = initConfig.nProcessThreadCnt;

	if ( InitializeSocket() == false )
		return false;

	if ( !CreateWorkerIOCP() )
		return false;

	if ( !CreateProcessIOCP() )
		return false;

	if ( !CreateWorkerThreads() )
		return false;

	if ( !CreateProcessThreads() )
		return false;

	if ( !CreateListenSock() )
		return false;

	initConfig.sockListener = GetListenSocket();
	initConfig.pIocpServer = this;

	m_vProcessPacket.resize( initConfig.nProcessPacketCnt, PROCESSPACKET{} );
	m_dwProcessPacketCnt = initConfig.nProcessPacketCnt;

	return true;
}

bool cIocpServer::ServerOff()
{
	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | cIocpServer::ServerOff() | 서버 종료를 시작합니다." ) );

	if ( m_hWorkerIOCP )
	{
		m_bWorkThreadFlag = false;
		for ( DWORD i = 0 ; i < m_dwWorkerThreadCount ; ++i )
		{
			PostQueuedCompletionStatus( m_hWorkerIOCP, 0, 0, nullptr );
		}
		CloseHandle( m_hWorkerIOCP );
		m_hWorkerIOCP = NULL;
	}

	if ( m_hProcessIOCP )
	{
		m_bProcessThreadFlag = false;
		for ( DWORD i = 0 ; i < m_dwProcessThreadCount ; ++i )
		{
			PostQueuedCompletionStatus( m_hProcessIOCP, 0, 0, nullptr );
		}
		CloseHandle( m_hProcessIOCP );
		m_hProcessIOCP = NULL;
	}

	for ( auto hWorkerThread : m_vWorkerThread )
	{
		if ( hWorkerThread != INVALID_HANDLE_VALUE )
			CloseHandle( hWorkerThread );
		hWorkerThread = INVALID_HANDLE_VALUE;
	}

	for ( auto hProcessThread : m_vProcessThread )
	{
		if ( hProcessThread != INVALID_HANDLE_VALUE )
			CloseHandle( hProcessThread );
		hProcessThread = INVALID_HANDLE_VALUE;
	}

	if ( m_ListenSock != INVALID_SOCKET )
	{
		closesocket( m_ListenSock );
		m_ListenSock = INVALID_SOCKET;
	}

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | sIocpServer::ServerOff() | 서버가 완전히 종료되었습니다." ) );

	return true;
}

bool cIocpServer::CreateProcessThreads()
{
	HANDLE	hThread;
	unsigned int uiThreadId = 0;

	m_bProcessThreadFlag = true;
	m_vProcessThread.reserve( m_dwProcessThreadCount );

	for ( DWORD dwCount = 0 ; dwCount < m_dwProcessThreadCount ; ++dwCount )
	{
		hThread = reinterpret_cast<HANDLE>( _beginthreadex( nullptr, 0, &CallProcessThread, this, CREATE_SUSPENDED, &uiThreadId ) );
		if ( NULL == hThread )
		{
			LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIocpServer::CreateProcessThreads() | _beginthreadex() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
			return false;
		}
		ResumeThread( hThread );
		SetThreadPriority( hThread, THREAD_PRIORITY_TIME_CRITICAL );

		m_vProcessThread.push_back( hThread );
	}

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | cIocpServer::CreateProcessThreads() | Process Thread 시작 ..." ) );

	return true;
}

bool cIocpServer::CreateWorkerThreads()
{
	unsigned int uiThreadId = 0;

	m_bWorkThreadFlag = true;
	m_vWorkerThread.reserve( m_dwWorkerThreadCount );

	for ( DWORD i = 0 ; i < m_dwWorkerThreadCount ; ++i )
	{
		HANDLE hThread = reinterpret_cast<HANDLE>( _beginthreadex( nullptr, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &uiThreadId ) );
		if ( NULL == hThread )
		{
			LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIocpServer::CreateWorkerThread() | _beginthreadex() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
			return false;
		}
		ResumeThread( hThread );

		m_vWorkerThread.push_back( hThread );
	}

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | cIocpServer::CreateWorkerThread() | Worker Thread 시작 ..." ) );

	return true;
}

void cIocpServer::GetProperThreadsCount()
{
	SYSTEM_INFO		SystemInfo;
	DWORD			ProperCount = 0;
	DWORD			DispatcherCount = 0;
	GetSystemInfo( &SystemInfo );
	ProperCount = SystemInfo.dwNumberOfProcessors * 2 + 1;
	if ( ProperCount > MAX_WORKER_THREAD )
		ProperCount = MAX_WORKER_THREAD;
	m_dwWorkerThreadCount = ProperCount;
}

bool cIocpServer::CreateWorkerIOCP()
{
	m_hWorkerIOCP = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
	if ( NULL == m_hWorkerIOCP ) {
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIocpServer::CreateWorkerIOCP() | CreateIoCompletionPort() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		return false;
	}

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | sIocpServer::CreateWorkerIOCP() | IOCP 객체 생성 성공" ) );

	return true;
}

bool cIocpServer::CreateProcessIOCP()
{
	m_hProcessIOCP = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
	if ( NULL == m_hProcessIOCP ) {
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIocpServer::CreateProcessIOCP() | CreateIoCompletionPort() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		return false;
	}

	LOG( eLogInfoType::LOG_INFO_LOW, _T( "SYSTEM | sIocpServer::CreateProcessIOCP() | IOCP 객체 생성 성공" ) );

	return true;
}

bool cIocpServer::CreateListenSock()
{
	int nRet;
	int nZero = 0;

	m_ListenSock = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED );
	if ( INVALID_SOCKET == m_ListenSock )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIocpServer::CreateListenSock() | Socket Creation Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		return false;
	}

	m_sockAddr.sin_family		= AF_INET;
	m_sockAddr.sin_port			= htons( m_usPort );
	m_sockAddr.sin_addr.s_addr	= htonl( INADDR_ANY );

	nRet = bind( m_ListenSock, (struct sockaddr*)&m_sockAddr, sizeof( m_sockAddr ) );
	if ( SOCKET_ERROR == nRet )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIocpServer::CreateListenSock() | bind() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		return false;
	}

	nRet = listen( m_ListenSock, 50 );
	if ( SOCKET_ERROR == nRet )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIocpServer::CreateListenSock() | listen() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		return false;
	}

	HANDLE hIOCPHandle;
	hIOCPHandle = CreateIoCompletionPort( (HANDLE)m_ListenSock, m_hWorkerIOCP, (DWORD)0, 0 );
	if ( NULL == hIOCPHandle || m_hWorkerIOCP != hIOCPHandle )
	{
		LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIocpServer::CreateListenSock() | CreateIoCompletionPort() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		return false;
	}

	return true;
}

void cIocpServer::WorkerThread()
{
	BOOL							bSuccess = FALSE;
	LPOVERLAPPED					lpOverlapped { nullptr };
	cConnection*					lpConnectionKey {};
	std::shared_ptr< cConnection >	lpConnection { nullptr };
	DWORD							dwIoSize = 0;

	while ( m_bWorkThreadFlag )
	{
		dwIoSize = 0;
		lpOverlapped = nullptr;
		bSuccess = GetQueuedCompletionStatus( m_hWorkerIOCP, &dwIoSize, reinterpret_cast<PULONG_PTR>( &lpConnectionKey ), &lpOverlapped, INFINITE );

		if ( !bSuccess )
		{
			LPOVERLAPPED_EX lpOverlappedEx = (LPOVERLAPPED_EX)lpOverlapped;
			if ( lpOverlapped == nullptr || lpOverlappedEx == nullptr )
			{
				LOG( eLogInfoType::LOG_ERROR_LOW, tstring{ _T( "SYSTEM | cIocpServer::WorkerThread() | GetQueuedCompletionStatus() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
				continue;
			}

			lpConnection = lpOverlappedEx->s_lpConnection;
			if ( lpConnection == nullptr )
				continue;

			if ( lpOverlappedEx->s_eOperation == eOperationType::OP_ACCEPT )
				lpConnection->DecrementAcceptIoRefCount();
			else if ( lpOverlappedEx->s_eOperation == eOperationType::OP_RECV )
				lpConnection->DecrementRecvIoRefCount();
			else if ( lpOverlappedEx->s_eOperation == eOperationType::OP_SEND )
				lpConnection->DecrementSendIoRefCount();

			CloseConnection( lpConnection );

			continue;
		}

		LPOVERLAPPED_EX lpOverlappedEx = (LPOVERLAPPED_EX)lpOverlapped;
		if ( nullptr == lpOverlappedEx )
			continue;

		switch ( lpOverlappedEx->s_eOperation )
		{

		case eOperationType::OP_ACCEPT:
			{
				DoAccept( lpOverlappedEx );
			}
			break;

		case eOperationType::OP_RECV:
			{
				DoRecv( lpOverlappedEx, dwIoSize );
			}
			break;

		case eOperationType::OP_SEND:
			{
				DoSend( lpOverlappedEx, dwIoSize );
			}
			break;

		}
	}
}


void cIocpServer::DoAccept( LPOVERLAPPED_EX lpOverlappedEx )
{
	SOCKADDR* lpLocalSockAddr = nullptr;
	SOCKADDR* lpRemoteSockAddr = nullptr;

	int nLocalSockAddrLen = 0, nRemoteSockAddrLen = 0;

	std::shared_ptr< cConnection > lpConnection { lpOverlappedEx->s_lpConnection };
	if ( lpConnection == nullptr )
		return;

	lpConnection->DecrementAcceptIoRefCount();

	GetAcceptExSockaddrs( lpConnection->m_szAddressBuf.data(), 0, sizeof( SOCKADDR_IN ) + 16, sizeof( SOCKADDR_IN ) + 16, &lpLocalSockAddr, &nLocalSockAddrLen, &lpRemoteSockAddr, &nRemoteSockAddrLen );
	if ( 0 != nRemoteSockAddrLen )
	{
		SOCKADDR_IN* pSIN = reinterpret_cast<SOCKADDR_IN*>( lpRemoteSockAddr );
		lpConnection->SetConnectionIp( pSIN );
	}
	else
	{
		LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cIocpServer::WorkerThread() | GetAcceptExSockaddrs() Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		CloseConnection( lpConnection );
		return;
	}

	if ( lpConnection->BindIOCP( m_hWorkerIOCP ) == false )
	{
		CloseConnection( lpConnection );
		return;
	}

	lpConnection->m_bIsConnect = true;
	if ( lpConnection->RecvPost( lpConnection->m_ringRecvBuffer.GetBeginMark(), 0 ) == false )
	{
		CloseConnection( lpConnection );
		return;
	}

	OnAccept( lpConnection );
}

void cIocpServer::DoRecv( LPOVERLAPPED_EX lpOverlappedEx, DWORD dwIoSize )
{
	std::shared_ptr< cConnection > lpConnection { lpOverlappedEx->s_lpConnection };
	if ( lpConnection == nullptr )
		return;
	lpConnection->DecrementRecvIoRefCount();

	int nMsgSize = 0, nRemain = 0;
	char* pCurrent = nullptr, *pNext = nullptr;

	nRemain = lpOverlappedEx->s_dwRemain;
	lpOverlappedEx->s_WsaBuf.buf = lpOverlappedEx->s_lpSocketMsg;
	lpOverlappedEx->s_dwRemain += dwIoSize;

	if ( lpOverlappedEx->s_dwRemain >= PACKET_SIZE_LENGTH )
		CopyMemory( &nMsgSize, &(lpOverlappedEx->s_WsaBuf.buf[ 0 ]), PACKET_SIZE_LENGTH );
	else
		nMsgSize = 0;

	if ( nMsgSize <= 0 || nMsgSize > lpConnection->m_ringRecvBuffer.GetBufferSize() )
	{
		LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cIocpServer::WorkerThread() | arrived wrong packet : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
		CloseConnection( lpConnection );
		return;
	}
	lpOverlappedEx->s_nTotalBytes = nMsgSize;

	if ( ( lpOverlappedEx->s_dwRemain < nMsgSize ) )
	{
		nRemain = lpOverlappedEx->s_dwRemain;
		pNext = lpOverlappedEx->s_WsaBuf.buf;
	}
	else
	{
		pCurrent = &(lpOverlappedEx->s_WsaBuf.buf[ 0 ]);
		int dwCurrentSize = nMsgSize;
		nRemain = lpOverlappedEx->s_dwRemain;
		if ( ProcessPacket( lpConnection, pCurrent, dwCurrentSize ) == false )
			return;

		nRemain -= dwCurrentSize;
		pNext = pCurrent + dwCurrentSize;

		while ( true )
		{
			if ( nRemain >= PACKET_SIZE_LENGTH )
			{
				CopyMemory( &nMsgSize, pNext, PACKET_SIZE_LENGTH );
				dwCurrentSize = nMsgSize;

				if ( nMsgSize <= 0 || nMsgSize > lpConnection->m_ringRecvBuffer.GetBufferSize() )
				{
					LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cIocpServer::WorkerThread() | arrived wrong packet : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
					CloseConnection( lpConnection );
					return;
				}
				lpOverlappedEx->s_nTotalBytes = dwCurrentSize;
				if ( nRemain >= dwCurrentSize )
				{
					if ( ProcessPacket( lpConnection, pNext, dwCurrentSize ) == false )
						return;
					nRemain -= dwCurrentSize;
					pNext += dwCurrentSize;
				}
				else
					break;
			}
			else
				break;
		}
	}

	lpConnection->RecvPost( pNext, nRemain );
}

void cIocpServer::DoSend( LPOVERLAPPED_EX lpOverlappedEx, DWORD dwIoSize )
{
	std::shared_ptr< cConnection > lpConnection { lpOverlappedEx->s_lpConnection };
	if ( lpConnection == nullptr )
		return;
	lpConnection->DecrementSendIoRefCount();

	lpOverlappedEx->s_dwRemain += dwIoSize;

	if ( lpOverlappedEx->s_nTotalBytes > lpOverlappedEx->s_dwRemain )
	{
		DWORD dwFlag = 0;
		DWORD dwSendNumBytes = 0;
		lpOverlappedEx->s_WsaBuf.buf += dwIoSize;
		lpOverlappedEx->s_WsaBuf.len -= dwIoSize;
		memset( &lpOverlappedEx->s_Overlapped, 0, sizeof( OVERLAPPED ) );
		lpConnection->IncrementSendIoRefCount();
		int nRet = WSASend( lpConnection->GetSocket(), &(lpOverlappedEx->s_WsaBuf), 1, &dwSendNumBytes, dwFlag, &(lpOverlappedEx->s_Overlapped), nullptr );
		if ( nRet == SOCKET_ERROR && ( WSAGetLastError() != ERROR_IO_PENDING ) )
		{
			lpConnection->DecrementSendIoRefCount();
			LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cIocpServer::WorkerThread() | WSASend Failed : ( " ) } + wsp::to( GetLastError() ) + _T( " )" ) );
			CloseConnection( lpConnection );
			return;
		}
	}
	else
	{
		lpConnection->m_ringSendBuffer.ReleaseBuffer( lpOverlappedEx->s_nTotalBytes );
		InterlockedExchange( (LPLONG)&lpConnection->m_bIsSend, TRUE );
		lpConnection->SendPost();
	}
}

LPPROCESSPACKET cIocpServer::GetProcessPacket( eOperationType operationType, LPARAM lParam, WPARAM wParam )
{
	DWORD dwProcessPacketCnt = InterlockedDecrement( (LPLONG)&m_dwProcessPacketCnt );
	if ( static_cast<int>( -1 ) == static_cast<int>( dwProcessPacketCnt ) )
	{
		InterlockedIncrement( (LPLONG)&m_dwProcessPacketCnt );
		LOG( eLogInfoType::LOG_ERROR_HIGH, _T( "SYSTEM | cIocpServer::GetProcessPacket() | ProcessPacket Empty ..." ) );
		return NULL;
	}
	LOG( eLogInfoType::LOG_INFO_LOW, wsp::to( dwProcessPacketCnt ) );
	LPPROCESSPACKET lpProcessPacket = &m_vProcessPacket[ m_dwProcessPacketCnt ];
	lpProcessPacket->s_eOperationType = operationType;
	lpProcessPacket->s_lParam = lParam;
	lpProcessPacket->s_wParam = wParam;
	return lpProcessPacket;
}

void cIocpServer::ClearProcessPacket( LPPROCESSPACKET lpProcessPacket )
{
	if ( nullptr != lpProcessPacket )
	{
		*lpProcessPacket = PROCESSPACKET{};
		InterlockedIncrement( (LPLONG)&m_dwProcessPacketCnt );
	}
}

bool cIocpServer::ProcessPacket( const std::shared_ptr< cConnection >& lpConnection, char* pCurrent, DWORD dwCurrentSize )
{
	int nUserBufSize = lpConnection->m_ringRecvBuffer.GetUsedBufferSize();

	if ( !OnRecvImmediately( lpConnection, dwCurrentSize, pCurrent ) )
	{
		LPPROCESSPACKET lpProcessPacket = GetProcessPacket( eOperationType::OP_RECVPACKET, (LPARAM)pCurrent, NULL );
		if ( nullptr == lpProcessPacket )
			return false;

		if ( 0 == PostQueuedCompletionStatus( m_hProcessIOCP, dwCurrentSize, reinterpret_cast<ULONG_PTR>( lpConnection.get() ), (LPOVERLAPPED)lpProcessPacket ) )
		{
			ClearProcessPacket( lpProcessPacket );
			LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cIocpServer::ProcessPacket() | PostQueuedCompletionStatus Failed : [ " ) } + wsp::to( GetLastError() ) + _T( " ], socket[ " ) + wsp::to( lpConnection->GetSocket() ) + _T( " ]" ) );
		}
	}
	else
		lpConnection->m_ringRecvBuffer.ReleaseBuffer( dwCurrentSize );

	return true;
}

bool cIocpServer::CloseConnection( const std::shared_ptr< cConnection >& lpConnection )
{
	if ( lpConnection->GetAcceptIoRefCount() != 0 || lpConnection->GetRecvIoRefCount() != 0 || lpConnection->GetSendIoRefCount() != 0 )
	{
		shutdown( lpConnection->GetSocket(), SD_BOTH );
		closesocket( lpConnection->GetSocket() );
		lpConnection->SetSocket( INVALID_SOCKET );
		return true;
	}

	if ( InterlockedCompareExchange( (LPLONG)&lpConnection->m_bIsClosed, TRUE, FALSE ) == FALSE )
	{
		LPPROCESSPACKET lpProcessPacket = GetProcessPacket( eOperationType::OP_CLOSE, NULL, NULL );
		if ( nullptr == lpProcessPacket )
			return false;

		if ( 0 == PostQueuedCompletionStatus( m_hProcessIOCP, 0, reinterpret_cast<ULONG_PTR>( lpConnection.get() ), (LPOVERLAPPED)lpProcessPacket ) )
		{
			ClearProcessPacket( lpProcessPacket );
			LOG( eLogInfoType::LOG_ERROR_NORMAL, tstring{ _T( "SYSTEM | cIocpServer::CloseConnection() | PostQueuedCompletionStatus Failed : [ " ) } + wsp::to( GetLastError() ) + _T( " ], socket[ " ) + wsp::to( lpConnection->GetSocket() ) + _T( " ]" ) );
			lpConnection->CloseConnection( true );
		}
	}

	return true;
}

void cIocpServer::ProcessThread()
{
	BOOL							bSuccess = FALSE;
	int								nRet = 0;
	LPPROCESSPACKET					lpProcessPacket = nullptr;
	LPOVERLAPPED					lpOverlapped = nullptr;
	cConnection*					lpConnectionKey {};
	std::shared_ptr< cConnection >	lpConnection { nullptr };
	LPOVERLAPPED_EX					lpOverlappedEx = nullptr;
	DWORD							dwIoSize = 0;

	while ( m_bProcessThreadFlag )
	{
		dwIoSize = 0;
		bSuccess = GetQueuedCompletionStatus( m_hProcessIOCP, &dwIoSize, reinterpret_cast<PULONG_PTR>( &lpConnectionKey ), (LPOVERLAPPED*)&lpProcessPacket, INFINITE );
		if ( TRUE == bSuccess && nullptr == lpConnectionKey )
			break;

		lpConnection = lpConnectionKey->shared_from_this();
		if ( nullptr == lpConnection )
			break;

		switch( lpProcessPacket->s_eOperationType )
		{

		case eOperationType::OP_CLOSE:
			{
				lpConnection->CloseConnection( true );
			}
			break;

		case eOperationType::OP_RECVPACKET:
			{
				if ( NULL == lpProcessPacket->s_lParam )
					continue;
				OnRecv( lpConnection, dwIoSize, (char*)lpProcessPacket->s_lParam );
				lpConnection->m_ringRecvBuffer.ReleaseBuffer( dwIoSize );
			}
			break;

		case eOperationType::OP_SYSTEM:
			{
				OnSystemMsg( lpConnection, static_cast<DWORD>( lpProcessPacket->s_lParam ), lpProcessPacket->s_wParam );
			}
			break;

		}

		ClearProcessPacket( lpProcessPacket );
	}
}
