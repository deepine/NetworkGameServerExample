#pragma once


#include	<array>
#include	<vector>
#include	<queue>
#include	"linkopt.h"
#include	"LibraryDef.h"
#include	"cMonitor.h"
#include	"cRingBuffer.h"


class NETLIB_API cConnection : public std::enable_shared_from_this< cConnection >, public cMonitor
{
public:
	static const size_t MAX_IP_LENGTH = 32;

public:
	cConnection();
	~cConnection();

	cConnection( const cConnection& rhs ) = delete;
	cConnection& operator=( const cConnection& rhs ) = delete;

public:
	void InitializeConnection();
	bool CreateConnection( const INITCONFIG& initConfig );
	bool CloseConnection( bool bForce = false );
	bool BindIOCP( HANDLE& hIOCP );
	bool RecvPost( char* pNext, DWORD dwRemain );
	bool SendPost();
	void SetSocket( SOCKET socket ) { m_socket = socket; }
	SOCKET GetSocket() const { return m_socket; }
	bool BindAcceptExSock();
	char* PrepareSendPacket( int slen );
	bool ReleaseRecvPacket();
	bool ReleaseSendPacket( LPOVERLAPPED_EX lpSendOverlappedEx = nullptr );

	void SetConnectionIp( SOCKADDR_IN* Ip );
	inline void SetConnectionIp( const tstring& sIp ) { m_szIp = sIp; }
	inline const tstring& GetConnectionIp() const { return m_szIp; }

	inline int GetIndex() { return m_nIndex; }

	inline int GetRecvBufSize() { return m_nRecvBufSize; }
	inline int GetSendBufSize() { return m_nSendBufSize; }

	cIocpServer* IocpServer() { return m_pIocpServer; }

	inline int GetRecvIoRefCount() { return m_dwRecvIoRefCount; }
	inline int GetSendIoRefCount() { return m_dwSendIoRefCount; }
	inline int GetAcceptIoRefCount() { return m_dwAcceptIoRefCount; }

	inline int IncrementRecvIoRefCount()
	{ return InterlockedIncrement( reinterpret_cast<LPLONG>( &m_dwRecvIoRefCount ) ); }
	inline int IncrementSendIoRefCount()
	{ return InterlockedIncrement( reinterpret_cast<LPLONG>( &m_dwSendIoRefCount ) ); }
	inline int IncrementAcceptIoRefCount()
	{ return InterlockedIncrement( reinterpret_cast<LPLONG>( &m_dwAcceptIoRefCount ) ); }
	inline int DecrementRecvIoRefCount()
	{ return ( m_dwRecvIoRefCount ? InterlockedDecrement( reinterpret_cast<LPLONG>( &m_dwRecvIoRefCount ) ) : 0 ); }
	inline int DecrementSendIoRefCount()
	{ return ( m_dwSendIoRefCount ? InterlockedDecrement( reinterpret_cast<LPLONG>( &m_dwSendIoRefCount ) ) : 0 ); }
	inline int DecrementAcceptIoRefCount()
	{ return ( m_dwAcceptIoRefCount ? InterlockedDecrement( reinterpret_cast<LPLONG>( &m_dwAcceptIoRefCount ) ) : 0 ); }

private:
	LPOVERLAPPED_EX m_lpRecvOverlappedEx;
	LPOVERLAPPED_EX m_lpSendOverlappedEx;

public:
	cRingBuffer m_ringRecvBuffer;
	cRingBuffer m_ringSendBuffer;

	std::array< char, 1024 > m_szAddressBuf;

	BOOL m_bIsClosed;
	bool m_bIsConnect;
	BOOL m_bIsSend;

private:
	SOCKET m_socket;
	int m_nRecvBufSize;
	int m_nSendBufSize;

	tstring m_szIp;
	int m_nIndex;

	cMonitor m_csConnection;

	SOCKET m_sockListener;
	HANDLE m_hIOCP;

	cIocpServer* m_pIocpServer;

	DWORD m_dwSendIoRefCount;
	DWORD m_dwRecvIoRefCount;
	DWORD m_dwAcceptIoRefCount;
};
