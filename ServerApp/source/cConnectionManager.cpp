
#include "stdafx.h"
#include "cConnectionManager.h"
#include "cLog.h"


cConnectionManager::~cConnectionManager()
{
	for ( auto pConnection : m_vConnection )
	{
		if ( pConnection )
		{
			delete pConnection;
		}
	}
}

bool cConnectionManager::CreateConnection( INITCONFIG& initConfig, DWORD dwMaxConnection )
{
	m_vConnection.resize( dwMaxConnection );

	for ( DWORD i = 0 ; i < dwMaxConnection ; ++i )
	{
		initConfig.nIndex = i;
		cConnection* pConnection { new cConnection };
		if ( !pConnection || pConnection->CreateConnection( initConfig ) == false )
			return false;

		m_vConnection[ i ] = pConnection;
	}

	return true;
}

bool cConnectionManager::AddConnection( cConnection* pConnection )
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

bool cConnectionManager::RemoveConnection( cConnection* pConnection )
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
}
