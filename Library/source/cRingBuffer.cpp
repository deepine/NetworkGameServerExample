
#include	"stdafx.h"
#include	"cRingBuffer.h"


cRingBuffer::cRingBuffer()
	: m_pRingBuffer{ nullptr }
	, m_pBeginMark{ nullptr }
	, m_pEndMark{ nullptr }
	, m_pCurrentMark{ nullptr }
	, m_pGettedBufferMark{ nullptr }
	, m_pLastMoveMark{ nullptr }
	, m_nUsedBufferSize{ 0 }
	, m_uiAllUserBufSize{ 0 }
{
}

cRingBuffer::~cRingBuffer()
{
	if ( nullptr != m_pBeginMark )
		delete[] m_pBeginMark;
}

bool cRingBuffer::Initialize()
{
	cMonitor::Owner lock{ m_csRingBuffer };

	m_nUsedBufferSize = 0;
	m_pCurrentMark = m_pBeginMark;
	m_pGettedBufferMark = m_pBeginMark;
	m_pLastMoveMark = m_pEndMark;
	m_uiAllUserBufSize = 0;

	return true;
}

bool cRingBuffer::Create( int nBufferSize )
{
	if ( nullptr != m_pBeginMark )
		delete[] m_pBeginMark;

	m_pBeginMark = new char[ nBufferSize ];

	if ( nullptr == m_pBeginMark )
		return false;

	m_pEndMark = m_pBeginMark + nBufferSize - 1;
	m_nBufferSize = nBufferSize;

	Initialize();

	return true;
}

char* cRingBuffer::ForwardMark( int nForwardLength )
{
	char* pPreCurrentMark = nullptr;

	cMonitor::Owner lock{ m_csRingBuffer };

	if ( m_nUsedBufferSize + nForwardLength > m_nBufferSize )
		return nullptr;

	if ( ( m_pEndMark - m_pCurrentMark ) >= nForwardLength )
	{
		pPreCurrentMark = m_pCurrentMark;
		m_pCurrentMark += nForwardLength;
	}
	else
	{
		m_pLastMoveMark = m_pCurrentMark;
		m_pCurrentMark = m_pBeginMark + nForwardLength;
		pPreCurrentMark = m_pBeginMark;
	}

	m_nUsedBufferSize += nForwardLength;
	m_uiAllUserBufSize += nForwardLength;

	return pPreCurrentMark;
}

char* cRingBuffer::ForwardMark( int nForwardLength, int nNextLength, DWORD dwRemainLength )
{
	cMonitor::Owner lock{ m_csRingBuffer };

	if ( ( m_nUsedBufferSize + nForwardLength + nNextLength ) > m_nBufferSize )
		return nullptr;

	if ( ( m_pEndMark - m_pCurrentMark ) > ( nNextLength + nForwardLength ) )
	{
		m_pCurrentMark += nForwardLength;
	}
	else
	{
		m_pLastMoveMark = m_pCurrentMark;
		CopyMemory( m_pBeginMark, m_pCurrentMark - ( dwRemainLength - nForwardLength ), dwRemainLength );
		m_pCurrentMark = m_pBeginMark + dwRemainLength;
	}

	m_nUsedBufferSize += nForwardLength;
	m_uiAllUserBufSize += nForwardLength;

	return m_pCurrentMark;
}

void cRingBuffer::ReleaseBuffer( int nReleaseSize )
{
	cMonitor::Owner lock{ m_csRingBuffer };

	m_nUsedBufferSize -= nReleaseSize;
}

char* cRingBuffer::GetBuffer( int nReadSize, int* pReadSize )
{
	char* pRet = nullptr;

	cMonitor::Owner lock{ m_csRingBuffer };

	if ( m_pLastMoveMark == m_pGettedBufferMark )
	{
		m_pGettedBufferMark = m_pBeginMark;
		m_pLastMoveMark = m_pEndMark;
	}

	if ( m_nUsedBufferSize > nReadSize )
	{
		if ( ( m_pLastMoveMark - m_pGettedBufferMark ) >= nReadSize )
		{
			*pReadSize = nReadSize;
			pRet = m_pGettedBufferMark;
			m_pGettedBufferMark += nReadSize;
		}
		else
		{
			*pReadSize = static_cast<int>( m_pLastMoveMark - m_pGettedBufferMark );
			pRet = m_pGettedBufferMark;
			m_pGettedBufferMark += *pReadSize;
		}
	}
	else if ( m_nUsedBufferSize > 0 )
	{
		if ( ( m_pLastMoveMark - m_pGettedBufferMark ) >= m_nUsedBufferSize )
		{
			*pReadSize = m_nUsedBufferSize;
			pRet = m_pGettedBufferMark;
			m_pGettedBufferMark += m_nUsedBufferSize;
		}
		else
		{
			*pReadSize = static_cast<int>( m_pLastMoveMark - m_pGettedBufferMark );
			pRet = m_pGettedBufferMark;
			m_pGettedBufferMark += *pReadSize;
		}
	}

	return pRet;
}
