
#include	"stdafx.h"
#include	"cLog.h"
#include	<time.h>
#include	"cSingleton.h"

tstring GetLogInfoTypeString( const eLogInfoType e )
{
	tstring s{};

	if ( eLogInfoType::LOG_NONE != ( e & eLogInfoType::LOG_ERROR_ALL ) )
	{
		if ( eLogInfoType::LOG_NONE != ( e & eLogInfoType::LOG_ERROR_CRITICAL ) )
		{
			s += _T( "LOG_ERROR_CRITICAL" );
		}
		else if ( eLogInfoType::LOG_NONE != ( e & eLogInfoType::LOG_ERROR_HIGH ) )
		{
			s += _T( "LOG_ERROR_HIGH" );
		}
		else if ( eLogInfoType::LOG_NONE != ( e & eLogInfoType::LOG_ERROR_NORMAL ) )
		{
			s += _T( "LOG_ERROR_NORMAL" );
		}
		else if ( eLogInfoType::LOG_NONE != ( e & eLogInfoType::LOG_ERROR_LOW ) )
		{
			s += _T( "LOG_ERROR_LOW" );
		}
	}

	if ( eLogInfoType::LOG_NONE != ( e & eLogInfoType::LOG_INFO_ALL ) )
	{
		tstring info;
		if ( eLogInfoType::LOG_NONE != ( e & eLogInfoType::LOG_INFO_CRITICAL ) )
		{
			info = _T( "LOG_INFO_CRITICAL" );
		}
		else if ( eLogInfoType::LOG_NONE != ( e & eLogInfoType::LOG_INFO_HIGH ) )
		{
			info = _T( "LOG_INFO_HIGH" );
		}
		else if ( eLogInfoType::LOG_NONE != ( e & eLogInfoType::LOG_INFO_NORMAL ) )
		{
			info = _T( "LOG_INFO_NORMAL" );
		}
		else if ( eLogInfoType::LOG_NONE != ( e & eLogInfoType::LOG_INFO_LOW ) )
		{
			info = _T( "LOG_INFO_LOW" );
		}

		if ( !info.empty() )
		{
			if ( !s.empty() )
			{
				s += _T( ", " );
			}
			s += info;
		}
	}

	if ( s.empty() )
	{
		s += _T( "LOG_NONE" );
	}

	return s;
}

void cLog::OnProcess()
{
	size_t nLogCount = m_queueLogMsg.size();
	for ( auto i = 0U ; i < nLogCount ; ++i )
	{
		auto pLogMsg = m_queueLogMsg.front();
		LogOutput( pLogMsg->s_eLogInfoType, pLogMsg->s_sOutputString );
		m_queueLogMsg.pop();
	}
}

bool cLog::Init( sLogConfig& LogConfig )
{
	CloseAllLog();

	const size_t STRLEN = 32;
	tchar strtime[ STRLEN ];
	time_t	curtime;
	struct tm loctime;
	curtime = time( NULL );
	localtime_s( &loctime, &curtime );

	m_vLogInfoTypes = LogConfig.s_vLogInfoTypes;

	_tcsftime( strtime, STRLEN, _T( "%Y년%m월%d일%H시%M분%S초" ), &loctime );

	CreateDirectory( _T( "./LOG" ), NULL );
	m_sLogFileName = wsp::to( _T( "./Log/" ) ) + LogConfig.s_sLogFileName + _T( "_" ) + strtime + _T( ".log" );
	m_sIP = LogConfig.s_sIP;
	m_sDSNNAME = LogConfig.s_sDSNNAME;
	m_sDSNID = LogConfig.s_sDSNID;
	m_sDSNPW = LogConfig.s_sDSNPW;
	m_eLogFileType = LogConfig.s_eLogFileType;
	m_nTCPPort = LogConfig.s_nTCPPort;
	m_nUDPPort = LogConfig.s_nUDPPort;
	m_nServerType = LogConfig.s_nServerType;
	m_dwFileMaxSize = LogConfig.s_dwFileMaxSize;

	m_hWnd = LogConfig.s_hWnd;
	bool bRet = false;

	do {
		if ( eLogInfoType::LOG_NONE != m_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_FILE ) ] )
			bRet = InitFile();
		if ( false == bRet )
			break;

		if ( eLogInfoType::LOG_NONE != m_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_DB ) ] )
			bRet = InitDB();
		if ( false == bRet )
			break;

		if ( eLogInfoType::LOG_NONE != m_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_UDP ) ] )
			bRet = InitUDP();
		if ( false == bRet )
			break;

		if ( eLogInfoType::LOG_NONE != m_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_TCP ) ] )
			bRet = InitTCP();
		if ( false == bRet )
			break;
	} while ( false );

	if ( !bRet ) {
		CloseAllLog();
		return false;
	}

	CreateThread( LogConfig.s_dwProcessTick );
	Run();

	return true;
}

void cLog::CloseAllLog()
{
	std::fill( m_vLogInfoTypes.begin(), m_vLogInfoTypes.end(), eLogInfoType::LOG_NONE );
	m_sLogFileName.clear();
	m_sIP.clear();
	m_sDSNNAME.clear();
	m_sDSNID.clear();
	m_sDSNPW.clear();
	m_nUDPPort = sLogConfig::DEFAULT_UDPPORT;
	m_nTCPPort = sLogConfig::DEFAULT_TCPPORT;
	m_eLogFileType = eLogFileType::FILETYPE_NONE;
	m_hWnd = NULL;
	m_nMsgBufferIdx = 0;

	if ( m_hLogFile )
	{
		CloseHandle( m_hLogFile );
		m_hLogFile = NULL;
	}

	if ( INVALID_SOCKET != m_sockUdp )
	{
		closesocket( m_sockUdp );
		m_sockUdp = INVALID_SOCKET;
	}

	if ( INVALID_SOCKET != m_sockTcp )
	{
		shutdown( m_sockTcp, SD_BOTH );
		closesocket( m_sockTcp );
		m_sockTcp = INVALID_SOCKET;
	}

	Stop();
}

bool cLog::InitDB()
{
	return true;
}

bool cLog::InitFile()
{
	m_hLogFile = CreateFile( m_sLogFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( NULL == m_hLogFile )
		return false;

	WORD wc = 0xFEFF;
	DWORD dwWrittenBytes = 0;
	SetFilePointer( m_hLogFile, 0, 0, FILE_END );
	BOOL hRet = WriteFile( m_hLogFile, &wc, sizeof( wc ), &dwWrittenBytes, NULL );
	if ( hRet == FALSE )
		return false;

	return true;
}

bool cLog::InitUDP()
{
	WSADATA wsaData;
	int nRet = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if ( nRet )
		return false;
	return true;
}

bool cLog::InitTCP()
{
	WSADATA wsaData;
	int nRet = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if ( nRet )
		return false;
	if ( INVALID_SOCKET != m_sockTcp )
		return false;

	sockaddr_in Addr;
	memset( reinterpret_cast<char*>( &Addr ), 0x00, sizeof( Addr ) );
	Addr.sin_family = AF_INET;
	InetPton( Addr.sin_family, m_sIP.c_str(), &Addr.sin_addr.s_addr );
	Addr.sin_port = htons( m_nTCPPort );
	m_sockTcp = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );

	nRet = connect( m_sockTcp, reinterpret_cast<sockaddr*>( &Addr ), sizeof( sockaddr ) );
	if ( SOCKET_ERROR == nRet )
		return false;

	return true;
}

void cLog::LogOutput( eLogInfoType eLogInfo, const tstring& sOutputString )
{
	if ( eLogInfoType::LOG_NONE != ( m_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_UDP ) ] & eLogInfo ) )
	{
		OutputUDP( eLogInfo, sOutputString );
	}
	if ( eLogInfoType::LOG_NONE != ( m_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_TCP ) ] & eLogInfo ) )
	{
		OutputTCP( eLogInfo, sOutputString );
	}

	const size_t STRLEN = 32;
	tchar strtime[ STRLEN ];
	time_t	curtime;
	struct tm loctime;

	const int nLogInfo = static_cast<int>( eLogInfo );
	int nIdx = nLogInfo;
	if ( 0 != ( nLogInfo >> 8 ) )
		nIdx = ( nLogInfo >> 8 ) + 0x20 - 3;
	else if ( 0 != ( nLogInfo >> 4 ) )
		nIdx = ( nLogInfo >> 4 ) + 0x10 - 1;

	if ( nIdx < 0 || nIdx > 31 )
		return;

	curtime = time( NULL );
	localtime_s( &loctime, &curtime );

	_tcsftime( strtime, STRLEN, _T( "%F_%T" ), &loctime );

	m_sOutStr = tstring{ strtime } + _T( " | " )
			+ ( ( nLogInfo >> 4 ) ? _T( "에러" ) : _T( "정보" ) ) + _T( " | " )
			+ GetLogInfoTypeString( eLogInfo ) + _T( " | " )
			+ sOutputString + _T( "\r\n" );

	if ( eLogInfoType::LOG_NONE != ( m_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_FILE ) ] & eLogInfo ) )
	{
		OutputFile( m_sOutStr );
	}
	if ( eLogInfoType::LOG_NONE != ( m_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_DB ) ] & eLogInfo ) )
	{
		OutputDB( m_sOutStr );
	}
	if ( eLogInfoType::LOG_NONE != ( m_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_WINDOW ) ] & eLogInfo ) )
	{
		OutputWindow( eLogInfo, m_sOutStr );
	}
	if ( eLogInfoType::LOG_NONE != ( m_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_OUTPUTWND ) ] & eLogInfo ) )
	{
		OutputOutputWnd( m_sOutStr );
	}
}

void cLog::OutputDB( const tstring& sOutputString )
{
	;
}

void cLog::OutputFile( const tstring& sOutputString )
{
	if ( NULL == m_hLogFile )
		return;

	DWORD dwWrittenBytes = 0;
	DWORD dwSize = 0;
	dwSize = GetFileSize( m_hLogFile, nullptr );

	if ( dwSize > m_dwFileMaxSize || dwSize > MAX_LOGFILE_SIZE )
	{
		const size_t STRLEN = 32;
		tchar strtime[ STRLEN ];
		time_t	curtime;
		struct tm loctime;

		curtime = time( NULL );
		localtime_s( &loctime, &curtime );

		_tcsftime( strtime, STRLEN, _T( "%Y년%m월%d일%H시%M분%S초" ), &loctime );
		m_sLogFileName.erase( m_sLogFileName.rfind( _T( "_" ) ) + 1 );
		m_sLogFileName += ( tstring{ strtime } + _T( ".log" ) );

		CloseHandle( m_hLogFile );
		m_hLogFile = NULL;
		InitFile();
	}

	SetFilePointer( m_hLogFile, 0, 0, FILE_END );
	BOOL hRet = WriteFile( m_hLogFile, sOutputString.c_str(), static_cast<DWORD>( sOutputString.size() * sizeof( tchar ) ), &dwWrittenBytes, NULL );
}

void cLog::OutputWindow( eLogInfoType eLogInfo, const tstring& sOutputString )
{
	if ( NULL == m_hWnd )
		return;
	//SendMessage( m_hWnd, WM_DEBUGMSG, reinterpret_cast<WPARAM>( sOutputString.c_str() ), reinterpret_cast<LPARAM>( eLogInfo ) );
}

void cLog::OutputOutputWnd( const tstring& sOutputString )
{
	OutputDebugString( sOutputString.c_str() );
}

void cLog::OutputUDP( eLogInfoType eLogInfo, const tstring& sOutputString )
{
	sockaddr_in Addr;
	memset( reinterpret_cast<char*>( &Addr ), 0x00, sizeof( Addr ) );
	Addr.sin_family = AF_INET;
	InetPton( Addr.sin_family, m_sIP.c_str(), &Addr.sin_addr.s_addr );
	Addr.sin_port = htons( m_nUDPPort );

	if ( INVALID_SOCKET == m_sockUdp )
	{
		m_sockTcp = socket( AF_INET, SOCK_DGRAM, IPPROTO_IP );
	}

	int iResult = sendto( m_sockUdp, reinterpret_cast<const char*>( sOutputString.c_str() ), static_cast<int>( sizeof( tstring::value_type ) * sOutputString.size() )
							, 0, reinterpret_cast<const struct sockaddr*>( &Addr ), sizeof( Addr ) );
}

void cLog::OutputTCP( eLogInfoType eLogInfo, const tstring& sOutputString )
{
	int iResult = send( m_sockTcp, reinterpret_cast<const char*>( sOutputString.c_str() ), static_cast<int>( sizeof( tstring::value_type ) * sOutputString.size() ), 0 );
}

void cLog::LogOutputLastErrorToMsgBox( const tstring& sOutputString )
{
	DWORD dwLastError = GetLastError();
	if ( dwLastError == 0 )
		return;
	LPVOID pDump;
	DWORD result;
	result = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, MAKELANGID( LANG_KOREAN, SUBLANG_KOREAN ), reinterpret_cast<LPTSTR>( &pDump ), 0, NULL );
	g_sOutStr = wsp::to( _T( "에러위치 : " ) ) + sOutputString + wsp::to( _T( "\n에러번호 : " ) ) + wsp::to( dwLastError ) + wsp::to( _T( "\n설명 : " ) ) + wsp::to( pDump );
	MessageBox( NULL, g_sOutStr.c_str(), _T( "GetLastError" ), MB_OK );

	if ( result )
		LocalFree( pDump );
}

bool INIT_LOG( sLogConfig& LogConfig )
{
	g_vLogMsg.resize( MAX_QUEUE_CNT );
	return cSingleton< cLog >::Get()->Init( LogConfig );
}

void LOG( eLogInfoType eLogInfoType, const tstring& s )
{
	LOG( eLogInfoType, { s } );
}

void LOG( eLogInfoType eLogInfoType, const std::initializer_list<tstring>& sl )
{
	cMonitor::Owner lock{ g_csLog };

	size_t nQueueCnt = cSingleton< cLog >::Get()->GetQueueSize();
	if ( MAX_QUEUE_CNT == nQueueCnt )
		return;

	g_vLogMsg[ nQueueCnt ].s_sOutputString.clear();
	for ( auto s : sl )
	{
		g_vLogMsg[ nQueueCnt ].s_sOutputString += s;
	}

	g_vLogMsg[ nQueueCnt ].s_eLogInfoType = eLogInfoType;

	cSingleton< cLog >::Get()->InsertMsgToQueue( &g_vLogMsg[ nQueueCnt ] );
}

void LOG_LASTERROR( const std::initializer_list<tstring>& sl )
{
	for ( auto s : sl )
	{
		g_sOutStr += s;
	}

	cSingleton< cLog >::Get()->LogOutputLastErrorToMsgBox( g_sOutStr );
}

void CLOSE_LOG()
{
	cSingleton< cLog >::Get()->CloseAllLog();
}
