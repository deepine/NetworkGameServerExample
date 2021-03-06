#pragma once


#include "cIocpServer.h"


class cIocpChatServer : public cIocpServer
{
public:
	cIocpChatServer() = default;
	//~cIocpChatServer();

	bool OnAccept( std::shared_ptr< cConnection > lpConnection ) override;
	bool OnRecv( std::shared_ptr< cConnection > lpConnection, DWORD dwSize, char* pRecvedMsg ) override;
	bool OnRecvImmediately( std::shared_ptr< cConnection > lpConnection, DWORD dwSize, char* pRecvedMsg ) override;
	void OnClose( std::shared_ptr< cConnection > lpConnection ) override;
	bool OnSystemMsg( std::shared_ptr< cConnection > lpConnection, DWORD dwMsgType, LPARAM lParam ) override;
};
