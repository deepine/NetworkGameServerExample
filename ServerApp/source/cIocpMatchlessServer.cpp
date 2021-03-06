
#include	"stdafx.h"
#include	"cIocpMatchlessServer.h"
#include	"cSingleton.h"
#include	"cConnection.h"
#include	"ServerAppRoot.h"
#include	"cConnectionManager.h"
#include	"cPacket.h"
#include	"LogUtil.h"


bool cIocpMatchlessServer::OnAccept( const std::shared_ptr< cConnection >& lpConnection )
{
	if ( nullptr == lpConnection )
	{
		WriteLog( tstring{ _T( "[ Error ] : Invalied Connection Request in OnAccept()" ) } );
	}

	cSingleton< cConnectionManager >::Get()->AddConnection( lpConnection );
	LOG( eLogInfoType::LOG_INFO_LOW, tstring{ _T( "SYSTEM | cIocpMatchlessServer::OnAccept() | IP[ " ) } /*+ lpConnection->GetConnectionIp()*/ + _T( " ] Socket[ " ) + wsp::to( lpConnection->GetSocket() ) + _T( " ] 접속 UserCnt[ " ) + wsp::to( cSingleton< cConnectionManager >::Get()->GetConnectionCnt() ) + _T( " ]" ) );

	ProcessClient_Accept( *lpConnection );

	return true;
}

bool cIocpMatchlessServer::OnRecv( const std::shared_ptr< cConnection >& lpConnection, DWORD dwSize, char* pRecvedMsg )
{
	return true;
}

bool cIocpMatchlessServer::OnRecvImmediately( const std::shared_ptr< cConnection >& lpConnection, DWORD dwSize, char* pRecvedMsg )
{
	if ( nullptr == lpConnection )
	{
		WriteLog( tstring{ _T( "[ Error ] : Invalied Connection Request in OnRecvImmediately()" ) } );
	}

	cIPacket iPacket;
	iPacket.SetBuffer( dwSize, pRecvedMsg );
	iPacket.ResetIndex();

	ProcessClient_Recv( *lpConnection, iPacket );

	return true;
}

void cIocpMatchlessServer::OnClose( const std::shared_ptr< cConnection >& lpConnection )
{
	if ( nullptr == lpConnection )
	{
		WriteLog( tstring{ _T( "[ Error ] : Invalied Connection Request in OnClose()" ) } );
	}

	cSingleton< cConnectionManager >::Get()->RemoveConnection( lpConnection );
	LOG( eLogInfoType::LOG_INFO_LOW, tstring{ _T( "SYSTEM | cIocpMatchlessServer::OnClose() | IP[ " ) } + lpConnection->GetConnectionIp() + _T( " ] Socket[ " ) + wsp::to( lpConnection->GetSocket() ) + _T( " ] 접속 종료 UserCnt [ " ) + wsp::to( cSingleton< cConnectionManager >::Get()->GetConnectionCnt() ) + _T( " ]" ) );
}

bool cIocpMatchlessServer::OnSystemMsg( const std::shared_ptr< cConnection >& lpConnection, DWORD dwMsgType, LPARAM lParam )
{
	return true;
}
