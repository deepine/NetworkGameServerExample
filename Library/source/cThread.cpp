
#include	"stdafx.h"
#include	"cThread.h"


cThread::cThread()
{
	m_hThread = NULL;
	m_bIsRun = false;
	m_dwWaitTick = 0;
	m_dwTickCount = 0;
	m_hQuitEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
}

cThread::~cThread()
{
	CloseHandle( m_hQuitEvent );
	if ( m_hThread )
		CloseHandle( m_hThread );
}

unsigned int WINAPI CallTickThread( LPVOID p )
{
	cThread* pTickThread = static_cast<cThread*>( p );
	if ( pTickThread )
		pTickThread->TickThread();

	return 1;
}

bool cThread::CreateThread( DWORD dwWaitTick )
{
	unsigned int uiThreadId = 0;
	m_hThread = reinterpret_cast<HANDLE>( _beginthreadex( NULL, 0, &CallTickThread, this, CREATE_SUSPENDED, &uiThreadId ) );

	if ( m_hThread == NULL )
	{
		//LOG( LOG_ERROR_NORMAL, "SYSTEM | cThread::CreateTickThread() | TickThread create failed : Error( %u )", GetLastError() );
		return false;
	}

	m_dwWaitTick = dwWaitTick;
	return true;
}

void cThread::Run()
{
	if ( false == m_bIsRun )
	{
		m_bIsRun = true;
		ResumeThread( m_hThread );
	}
}

void cThread::Stop()
{
	if ( true == m_bIsRun )
	{
		m_bIsRun = false;
		SuspendThread( m_hThread );
	}
}

void cThread::TickThread()
{
	while ( true )
	{
		DWORD dwRet = WaitForSingleObject( m_hQuitEvent, m_dwWaitTick );
		if ( WAIT_OBJECT_0 == dwRet )
			break;
		else if ( WAIT_TIMEOUT == dwRet )
		{
			++m_dwTickCount;
			OnProcess();
		}
	}
}

void cThread::DestroyThread()
{
	Run();
	SetEvent( m_hQuitEvent );
	WaitForSingleObject( m_hThread, INFINITE );
}
