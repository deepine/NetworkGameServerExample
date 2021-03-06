// ChatServerDemo.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "cLog.h"
#include "cIocpChatServer.h"
#include "LibraryDef.h"
#include "cSingleton.h"
#include "cConnectionManager.h"


void StartServer()
{
	sLogConfig LogConfig;
	LogConfig.s_sLogFileName = _T( "ChatServer" );
	//LogConfig.s_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_OUTPUTWND ) ] = eLogInfoType::LOG_ALL;
	//LogConfig.s_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_WINDOW ) ] = eLogInfoType::LOG_ALL;
	LogConfig.s_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_FILE ) ] = eLogInfoType::LOG_ALL;
	//LogConfig.s_hWnd = m_hWnd;
	INIT_LOG( LogConfig );

	INITCONFIG InitConfig;
	InitConfig.nServerPort = 8080;
	InitConfig.nRecvBufCnt = 10;
	InitConfig.nRecvBufSize = 1024;
	InitConfig.nProcessPacketCnt = 100;
	InitConfig.nSendBufCnt = 10;
	InitConfig.nSendBufSize = 1024;
	InitConfig.nWokerThreadCnt = 2;
	InitConfig.nProcessThreadCnt = 1;

	cSingleton< cIocpChatServer >::Get()->ServerStart( InitConfig );
	cSingleton< cConnectionManager >::Get()->CreateConnection( InitConfig, 10 );
	LOG( eLogInfoType::LOG_INFO_LOW, tstring{ _T( "SYSTEM | ServerStart() | 서버 시작 ..." ) } );
}


int main()
{
	StartServer();

    return 0;
}

