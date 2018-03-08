#pragma once


#include "cMonitor.h"
#include "cConnection.h"


class cConnectionManager
{
public:
	~cConnectionManager();

	using CONN_MAP = std::map< cConnection*, DWORD >;

	bool CreateConnection( INITCONFIG& initConfig, DWORD dwMaxConnection );
	bool AddConnection( cConnection* pConnection );
	bool RemoveConnection( cConnection* pConnection );
	inline int __fastcall GetConnectionCnt()
	{ return m_mapConnection.size(); }

	void BroadCast_Chat( const tstring& szIP, const tstring& szChatMsg );

protected:
	CONN_MAP						m_mapConnection;
	std::vector< cConnection* >		m_vConnection;
	cMonitor						m_csConnection;
};
