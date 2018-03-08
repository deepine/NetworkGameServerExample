#pragma once


#include	<vector>

#include	"linkopt.h"
#include	"LibraryDef.h"
#include	"cMonitor.h"


class cConnection;


class NETLIB_API cIocpServer : public cMonitor
{
public:
	cIocpServer() = default;
	//~cIocpServer();

	cIocpServer( const cIocpServer& rhs ) = delete;
	cIocpServer& operator=( const cIocpServer& rhs ) = delete;

	bool InitializeSocket();

	void WorkerThread();
	void ProcessThread();
	bool CloseConnection( cConnection* lpConnection );
	bool ProcessPacket( cConnection* lpConnection, char* pCurrent, DWORD dwCurrentSize );

	virtual bool ServerStart( INITCONFIG& initConfig );
	virtual bool ServerOff();
	SOCKET GetListenSocket() { return m_ListenSock; }
	unsigned short GetServerPort() { return m_usPort; }
	//char* GetServerIp() { return m_szIp; }
	const std::vector< char >& GetServerIp() { return m_szIp; }
	inline HANDLE GetWorkerIOCP() { return m_hWorkerIOCP; }
	void DoAccept( LPOVERLAPPED_EX lpOverlappedEx );
	void DoRecv( LPOVERLAPPED_EX lpOverlappedEx, DWORD dwIoSize );
	void DoSend( LPOVERLAPPED_EX lpOverlappedEx, DWORD dwIoSize );
	LPPROCESSPACKET GetProcessPacket( eOperationType operationType, LPARAM lParam, WPARAM wParam );
	void ClearProcessPacket( LPPROCESSPACKET lpProcessPacket );

	virtual bool OnAccept( cConnection* lpConnection ) = 0;
	virtual bool OnRecv( cConnection* lpConnection, DWORD dwSize, char* pRecvedMsg ) = 0;

	virtual bool OnRecvImmediately( cConnection* lpConnection, DWORD dwSize, char* pRecvedMsg ) = 0;

	virtual void OnClose( cConnection* lpConnection ) = 0;
	virtual bool OnSystemMsg( cConnection* lpConnection, DWORD dwMsgType, LPARAM lParam ) = 0;

	static cIocpServer* GetIocpServer() { return cIocpServer::m_pIocpServer; }
	static cIocpServer* m_pIocpServer;

private:
	bool CreateProcessThreads();
	bool CreateWorkerThreads();
	void GetProperThreadsCount();
	bool CreateWorkerIOCP();
	bool CreateProcessIOCP();
	bool CreateListenSock();

private:
	SOCKET m_ListenSock;

	HANDLE m_hWorkerIOCP;
	HANDLE m_hProcessIOCP;

	std::vector< HANDLE > m_vWorkerThread;
	std::vector< HANDLE > m_vProcessThread;

	unsigned short m_usPort;
	std::vector< char > m_szIp;

	DWORD m_dwTimeTick;
	DWORD m_dwWorkerThreadCount;
	DWORD m_dwProcessThreadCount;

	bool m_bWorkThreadFlag;
	bool m_bProcessThreadFlag;

	std::vector< PROCESSPACKET > m_vProcessPacket;
	DWORD m_dwProcessPacketCnt;
};

inline cIocpServer* IocpServer()
{
	return cIocpServer::GetIocpServer();
}
