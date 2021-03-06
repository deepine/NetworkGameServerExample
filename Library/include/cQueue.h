#pragma once


template < typename TYPE >
class cQueue : public cMonitor
{
	static const int MAX_QUEUESIZE = 128;

public:
	cQueue( int nMaxSize = MAX_QUEUESIZE );
	~cQueue();

	bool PushQueue( TYPE typeQueueItem );
	void PopQueue();
	bool IsEmptyQueue();
	TYPE GetFrontQueue();
	int GetQueueSize();
	int GetQueueMaxSize() { return m_nQueueMaxSize; }
	void SetQueueMaxSize( int nMaxSize )
	{ m_nQueueMaxSize = nMaxSize; }
	void ClearQueue();

private:
	TYPE* m_arrQueue;
	int m_nQueueMaxSize;
	cMonitor m_csQueue;

	int m_nCurSize;
	int m_nEndMark;
	int m_nBeginMark;
};

template < typename TYPE >
cQueue< TYPE >::cQueue( int nMaxSize )
{
	m_arrQueue = new TYPE[ nMaxSize ];
	m_nQueueMaxSize = nMaxSize;
	ClearQueue();
}

template < typename TYPE >
cQueue< TYPE >::~cQueue< TYPE >()
{
	delete[] m_arrQueue;
}


template < typename TYPE >
bool cQueue< TYPE >::PushQueue( TYPE typeQueueItem )
{
	cMonitor::Owner lock{ m_csQueue };
	{
		if ( m_nCurSize >= m_nQueueMaxSize )
			return false;

		++m_nCurSize;

		if ( m_nEndMark == m_nQueueMaxSize )
			m_nEndMark = 0;
		m_arrQueue[ m_nEndMark++ ] = typeQueueItem;
	}

	return true;
}

template < typename TYPE >
TYPE cQueue< TYPE >::GetFrontQueue()
{
	TYPE typeQueueItem;
	cMonitor::Owner lock{ m_csQueue };
	{
		if ( m_nCurSize <= 0 )
			return NULL;
		if ( m_nBeginMark == m_nQueueMaxSize )
			m_nBeginMark = 0;
		typeQueueItem = m_arrQueue[ m_nBeginMark ];
	}
	return typeQueueItem;
}

template < typename TYPE >
void cQueue< TYPE >::PopQueue()
{
	cMonitor::Owner lock{ m_csQueue };
	{
		--m_nCurSize;
		++m_nBeginMark;
	}
}

template < typename TYPE >
bool cQueue< TYPE >::IsEmptyQueue()
{
	bool bFlag = false;
	cMonitor::Owner lock{ m_csQueue };
	{
		bFlag = ( m_nCurSize > 0 ) ? false : true;
	}

	return bFlag;
}

template < typename TYPE >
int cQueue< TYPE >::GetQueueSize()
{
	int nSize;
	cMonitor::Owner lock{ m_csQueue };
	{
		nSize = m_nCurSize;
	}

	return nSize;
}

template < typename TYPE >
void cQueue< TYPE >::ClearQueue()
{
	cMonitor::Owner lock{ m_csQueue };
	{
		m_nCurSize = 0;
		m_nEndMark = 0;
		m_nBeginMark = 0;
	}
}
