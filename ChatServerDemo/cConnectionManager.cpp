
#include "stdafx.h"
#include "cConnectionManager.h"
#include "ChatServerDemoDef.h"
#include "cLog.h"


bool cConnectionManager::CreateConnection( INITCONFIG& initConfig, DWORD dwMaxConnection )
{
	m_vConnection.resize( dwMaxConnection );

	for ( int i = 0 ; i < dwMaxConnection ; ++i )
	{
		initConfig.nIndex = i;
		std::shared_ptr< cConnection > pConnection { new cConnection };
		if ( !pConnection || pConnection->CreateConnection( initConfig ) == false )
			return false;

		m_vConnection[ i ] = pConnection;
	}

	return true;
}

bool cConnectionManager::AddConnection( std::shared_ptr< cConnection > pConnection )
{
	cMonitor::Owner lock { m_csConnection };
	auto conn_it = m_mapConnection.find( pConnection );
	if ( conn_it != m_mapConnection.end() )
	{
		LOG( eLogInfoType::LOG_INFO_NORMAL, tstring{ _T( "SYSTEM | cConnectionManager::AddConnection() | index[ " ) } + wsp::to( pConnection->GetIndex() ) + _T( " ]는 이미 Connection map 에 있습니다." ) );
		return false;
	}

	m_mapConnection.insert( std::make_pair( pConnection, GetTickCount() ) );

	return true;
}

bool cConnectionManager::RemoveConnection( std::shared_ptr< cConnection > pConnection )
{
	cMonitor::Owner lock { m_csConnection };
	auto conn_it = m_mapConnection.find( pConnection );
	if ( conn_it == m_mapConnection.end() )
	{
		LOG( eLogInfoType::LOG_INFO_NORMAL, tstring{ _T( "SYSTEM | cConnectionManager::RemoveConnection() | index[ " ) } + wsp::to( pConnection->GetIndex() ) + _T( " ]는 Connection map 에 없습니다." ) );
		return false;
	}

	m_mapConnection.erase( pConnection );

	return true;
}

void cConnectionManager::BroadCast_Chat( const tstring& szIP, const tstring& szChatMsg )
{
	cMonitor::Owner lock { m_csConnection };
	for ( auto it : m_mapConnection )
	{
		auto pConnection = it.first;
		auto pChat = reinterpret_cast<Packet_Chat*>( pConnection->PrepareSendPacket( sizeof( Packet_Chat ) ) );
		if ( nullptr == pChat )
			continue;
		pChat->s_sType = PACKET_CHAT;
		pChat->s_szChatMsg = szChatMsg;
		pChat->s_szIP = szIP;
		pConnection->SendPost();
	}
}
