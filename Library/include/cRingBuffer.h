#pragma once


#include	"linkopt.h"

#include	"cMonitor.h"


class NETLIB_API cRingBuffer : public cMonitor
{
public:
	static const int MAX_RINGBUFSIZE = 4 * 1024;

public:
	cRingBuffer();
	~cRingBuffer();

	cRingBuffer( const cRingBuffer& rhs ) = delete;
	cRingBuffer& operator=( const cRingBuffer& rhs ) = delete;

	bool Create( int nBufferSize = MAX_RINGBUFSIZE );
	bool Initialize();
	inline int GetBufferSize() { return m_nBufferSize; }

	inline char* GetBeginMark() { return m_pBeginMark; }
	inline char* GetCurrentMark() { return m_pCurrentMark; }
	inline char* GetEndMark() { return m_pEndMark; }

	char* ForwardMark( int nForwardLength );
	char* ForwardMark( int nForwardLength, int nNextLength, DWORD dwRemainLength );

	void ReleaseBuffer( int nReleaseSize );
	int GetUsedBufferSize() { return m_nUsedBufferSize; }
	int GetAllUsedBufferSize() { return m_uiAllUserBufSize; }

	char* GetBuffer( int nReadSize, int* pReadSize );

private:
	char* m_pRingBuffer;

	char* m_pBeginMark;
	char* m_pEndMark;
	char* m_pCurrentMark;
	char* m_pGettedBufferMark;
	char* m_pLastMoveMark;

	int m_nBufferSize;
	int m_nUsedBufferSize;
	int m_uiAllUserBufSize;

	cMonitor m_csRingBuffer;
};
