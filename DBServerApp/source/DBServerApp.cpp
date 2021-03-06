// DBServerApp.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include	"stdafx.h"

#include	"LibraryDef.h"
#include	"cException.h"
#include	"cLog.h"
#include	"CMiniDump.h"
#include	"cSingleton.h"
#include	"cSQLManager.h"
#include	"MatchlessProtocol.h"
#include	"cIocpMatchlessDBServer.h"
#include	"cServerConnectionManager.h"
#include	"LogUtil.h"
#include	"cNetMessageHandlerManager.h"


int OutputServerInitialInfo( const SOCKADDR_IN & aAddrInfo, const SOCKET aListenSocket )
{
	std::vector< tchar > vHostName( NI_MAXHOST );
	std::vector< tchar > vServInfo( NI_MAXSERV );
	int	addrlen;
	SOCKADDR_IN	tempAddr_in;

	DWORD dwRet = GetNameInfo( (SOCKADDR*)&aAddrInfo, sizeof( SOCKADDR ), vHostName.data(), NI_MAXHOST, vServInfo.data(), NI_MAXSERV, NI_NUMERICSERV );
	if( 0 != dwRet )
	{
		WriteLog( _T( "-- Failed get server information." ) );
		return -1;
	}

	addrlen = sizeof( tempAddr_in );
	if( SOCKET_ERROR == getsockname( aListenSocket, (SOCKADDR*)&tempAddr_in, &addrlen ) )
	{
		WriteLog( _T( "-- Failed get server information." ) );
		return -1;
	}

	WriteLog( _T( "-- Succeed launch server." ) );

	WriteLog( tstring{ _T( "-- Server IP address : " ) } + Inet_Ntop( tempAddr_in ) + _T( ", " ) + tstring{ vHostName.data() } );

	WriteLog( tstring{ _T( "-- Server port number : " ) } + wsp::to( ntohs( aAddrInfo.sin_port ) ) );

	return 0;
}


int Startup()
{
	CMiniDump::Begin();

	if ( InitLog() )
	{
		throw cException_FailedToCreateObject{ "Log", __FILE__, __LINE__ };
	}


	cSingleton< cNetMessageHandlerManager >::Get()->Initialize();


	int nReturnCode = RETURN_SUCCEED;

	cSQLManager::INITCONFIG		sqlConfig;
	sqlConfig.sDNS = _T( "MatchlessGame" );
	sqlConfig.sUser = _T( "sa" );
	sqlConfig.sAuth = _T( "root" );
	if ( nReturnCode = cSingleton< cSQLManager >::Get()->Initialize( sqlConfig ) )
	{
		WriteLog( { _T( "Failed to SQL Manager Initialize : cSQLManager::Initialize(), Code Num : " ), wsp::to( nReturnCode ) }, { eLogInfoType::LOG_ERROR_HIGH } );
		throw cException_FailedToCreateObject{ "SQL Manager", __FILE__, __LINE__ };
	}


	SYSTEM_INFO si;
	GetSystemInfo( &si );

	INITCONFIG initConfig {};
	initConfig.nServerPort = MATCHLESS_DBSERVER_PORT;
	initConfig.nRecvBufCnt = 10;
	initConfig.nRecvBufSize = 1024;
	initConfig.nProcessPacketCnt = 100;
	initConfig.nSendBufCnt = 10;
	initConfig.nSendBufSize = 1024;
	initConfig.nWokerThreadCnt = si.dwNumberOfProcessors;
	initConfig.nProcessThreadCnt = 1;

	cIocpMatchlessDBServer* pServer = cSingleton< cIocpMatchlessDBServer >::Get();
	pServer->ServerStart( initConfig );

	OutputServerInitialInfo( pServer->GetSockAddr(), pServer->GetListenSocket() );

	cSingleton< cServerConnectionManager >::Get()->CreateConnection( initConfig, 0 );


	return RETURN_SUCCEED;
}

int Run()
{
	//auto tpnow = std::chrono::system_clock::now();
	//auto tt = std::chrono::system_clock::to_time_t( tpnow );
	//tm tempTM;
	//auto tm = localtime_s( &tempTM, &tt );

	//std::vector< char > vStr( 40 );
	//std::strftime( vStr.data(), vStr.size(), "%F %T", &tempTM );

	//tstring sSQLCmd{};
	//sSQLCmd = _T( "INSERT INTO [dbo].[LogCommon] ([Time],[Type],[Log]) VALUES ('" );
	//sSQLCmd += tstring{ vStr.begin(), vStr.end() }.c_str();
	//sSQLCmd += _T( "', 0, 'For Test~!')" );
	//cSingleton< cSQLManager >::Get()->Execute( sSQLCmd );

	std::vector< tchar > vInput( 1000 );

	_tprintf( _T( "Enter SQL commands, type (control)Z to exit\nSQL COMMAND>" ) );

	while ( _fgetts( vInput.data(), static_cast<int>( vInput.size() - 1 ), stdin ) )
	{
		if ( !(*vInput.data()) )
		{
			_tprintf( _T( "SQL COMMAND>" ) );
			continue;
		}

		cSingleton< cSQLManager >::Get()->Execute( tstring{ vInput.begin(), vInput.end() }.c_str() );

		_tprintf( _T( "SQL COMMAND>" ) );
	}

	return RETURN_SUCCEED;
}

int Cleanup()
{
	cSingleton< cSQLManager >::Get()->Shutdown();

	CloseLog();

	CMiniDump::End();

	return RETURN_SUCCEED;
}


int main()
try {
	if ( Startup() )
	{
		WriteLog( _T( "Failed DBServerApp Startup()" ), { eLogInfoType::LOG_ERROR_HIGH } );
		return 1;
	}

	if ( Run() )
	{
		WriteLog( _T( "Occur Error while DBServerApp Run()" ), { eLogInfoType::LOG_ERROR_HIGH } );
		return 10000;
	}

	Cleanup();

    return RETURN_SUCCEED;
}
catch ( const cException& e ) {
	tstring notice;

	notice = wsp::to( e.what() );

	WriteLog( notice, { eLogInfoType::LOG_ERROR_HIGH } );

	return 0xa0000003;
}
catch ( const std::runtime_error& e ) {
	tstring notice;

	notice = wsp::to( e.what() );

	WriteLog( notice, { eLogInfoType::LOG_ERROR_HIGH } );

	return 0xa0000002;
}
catch ( const std::exception& e ) {
	tstring notice;

	notice = wsp::to( e.what() );

	WriteLog( notice, { eLogInfoType::LOG_ERROR_HIGH } );

	return 0xa0000001;
}
catch ( ... ) {
	WriteLog( _T( "Unknown Exception" ), { eLogInfoType::LOG_ERROR_HIGH } );

	return 0xa0000000;
}
