
#include	"stdafx.h"
#include	"debugconsole.h"



DebugConsole::DebugConsole()
{
	//AllocConsole�Լ��� �ڽ��� ������ �ܿ� �ܼ� ���μ����� �ϳ� �� �۵���Ŵ
	if( !AllocConsole() )
	{
		MessageBox( NULL, TEXT( "AllocConsole Error!!!" ), TEXT( "Error" ), MB_OK );
		m_isAllocated = FALSE;
	}
	else
	{
		//m_hConsoleOutput�� �޾ƿ� �ܼ� �ƿ�ǲ �ڵ��� ��
		m_hConsoleOutput = GetStdHandle( STD_OUTPUT_HANDLE );
		m_isAllocated = TRUE;
	}
}

DebugConsole::~DebugConsole()
{
	//�����ڰ� ȣ��Ǿ����� �ܼ� ���μ����� �������־�� ��
	if( !FreeConsole() )
	{
		MessageBox( NULL, TEXT( "FreeConsole Error!!!" ), TEXT( "Error" ), MB_OK );
	}
}

void DebugConsole::Output( char * fmt, ... )
{
	va_list					argptr;
	char					cBuf[512];
	int						iCnt;
	DWORD					dwWritten;


	if( !m_isAllocated ) return;

	va_start( argptr, fmt );
	iCnt = vsprintf( cBuf, fmt, argptr );
	va_end( argptr );

	//�ֿܼ��ٰ� ��� ���ִ� �Լ��� ����
	WriteConsoleA( m_hConsoleOutput, cBuf, iCnt, &dwWritten, NULL );	
}
