
#include "stdafx.h"
#include "cIocpMatchlessServer.h"
#include "cSingleton.h"
#include "cConnection.h"
#include "ServerAppRoot.h"
#include "cConnectionManager.h"


bool cIocpMatchlessServer::OnAccept( cConnection* lpConnection )
{
	cSingleton< cConnectionManager >::Get()->AddConnection( lpConnection );
	LOG( eLogInfoType::LOG_INFO_LOW, tstring{ _T( "SYSTEM | cIocpChatServer::OnAccept() | IP[ " ) } /*+ lpConnection->GetConnectionIp()*/ + _T( " ] Socket[ " ) + wsp::to( lpConnection->GetSocket() ) + _T( " ] 접속 UserCnt[ " ) + wsp::to( cSingleton< cConnectionManager >::Get()->GetConnectionCnt() ) + _T( " ]" ) );

	ProcessClient_Accept( reinterpret_cast<LPVOID>( lpConnection->GetSocket() ) );

	return true;
}

bool cIocpMatchlessServer::OnRecv( cConnection* lpConnection, DWORD dwSize, char* pRecvedMsg )
{
	return true;
}

bool cIocpMatchlessServer::OnRecvImmediately( cConnection* lpConnection, DWORD dwSize, char* pRecvedMsg )
{
	CNetMessage message;
	message.SetData( pRecvedMsg, dwSize );

	ProcessClient_Recv( lpConnection->GetSocket(), message );

	return true;
}

void cIocpMatchlessServer::OnClose( cConnection* lpConnection )
{
	;
}

bool cIocpMatchlessServer::OnSystemMsg( cConnection* lpConnection, DWORD dwMsgType, LPARAM lParam )
{
	return true;
}
