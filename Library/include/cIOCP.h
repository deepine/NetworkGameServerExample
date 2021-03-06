#pragma once


#include <vector>
#include "linkopt.h"


const size_t MAX_SOCKBUF = 1024;
const size_t MAX_CLIENT = 100;
const size_t MAX_WORKERTHREAD = 4;


enum class eOperation
{
	OP_RECV,
	OP_SEND
};


struct stOverlappedEx
{
	WSAOVERLAPPED	m_wsaOverlapped;
	SOCKET			m_socketClient;
	WSABUF			m_wsaBuf;
	char			m_szBuf[ MAX_SOCKBUF ];
	eOperation		m_eOperation;
};

struct stClientInfo
{
	SOCKET			m_socketClient;
	stOverlappedEx	m_stRecvOverlappedEx;
	stOverlappedEx	m_stSendOverlappedEx;
};


class NETLIB_API cIOCP
{
public:
	cIOCP();
	~cIOCP();

	bool InitSocket();
	void CloseSocket( stClientInfo& clientInfo, const bool bIsForce = false );

	bool BindandListen( int nBindPort );
	bool StartServer();
	bool CreateWorkerThread();
	bool CreateAccepterThread();
	stClientInfo* GetEmptyClientInfo();

	bool BindIOCompletionPort( const stClientInfo& clientInfo );
	bool BindRecv( stClientInfo& clientInfo );
	bool SendMsg( stClientInfo& clientInfo, char* pMsg, int nLen );

	void WorkerThread();
	void AccepterThread();

	void DestroyThread();

private:
	std::vector< stClientInfo >		m_vClientInfo;
	SOCKET							m_socketListen;
	int								m_nClientCnt;

	std::vector< HANDLE >			m_vWorkerThread;
	HANDLE							m_hAccepterThread;
	HANDLE							m_hIOCP;
	bool							m_bWorkerRun;
	bool							m_bAccepterRun;
	char							m_szSocketBuf[ 1024 ];
};
