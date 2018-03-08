#pragma once


#include "cIocpServer.h"


class cIocpMatchlessServer : public cIocpServer
{
public:
	bool OnAccept( cConnection* lpConnection ) override;
	bool OnRecv( cConnection* lpConnection, DWORD dwSize, char* pRecvedMsg ) override;
	bool OnRecvImmediately( cConnection* lpConnection, DWORD dwSize, char* pRecvedMsg ) override;
	void OnClose( cConnection* lpConnection ) override;
	bool OnSystemMsg( cConnection* lpConnection, DWORD dwMsgType, LPARAM lParam ) override;
};
