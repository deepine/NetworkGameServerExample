#pragma once


#include "cMonitor.h"
#include "cConnection.h"


class cConnectionManager : public cMonitor
{
public:
	//cConnectionManager();
	//~cConnectionManager();

	using CONN_MAP = std::map< std::shared_ptr< cConnection >, DWORD >;

	bool CreateConnection( INITCONFIG& initConfig, DWORD dwMaxConnection );
	bool AddConnection( std::shared_ptr< cConnection > pConnection );
	bool RemoveConnection( std::shared_ptr< cConnection > pConnection );
	inline int __fastcall GetConnectionCnt()
	{ return m_mapConnection.size(); }

	void BroadCast_Chat( const tstring& szIP, const tstring& szChatMsg );

protected:
	CONN_MAP										m_mapConnection;
	std::vector< std::shared_ptr< cConnection > >	m_vConnection;
	cMonitor										m_csConnection;
};
