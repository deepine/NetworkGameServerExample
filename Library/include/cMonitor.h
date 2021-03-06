#pragma once


#include	"linkopt.h"


class NETLIB_API cMonitor
{
public:
	class NETLIB_API Owner
	{
	public:
		Owner( cMonitor& crit );
		~Owner();

		Owner( const Owner& rhs ) = delete;
		Owner& operator=( const Owner& rhs ) = delete;

	private:
		cMonitor& m_csSyncObject;
	};

	cMonitor();
	~cMonitor();

	cMonitor( const cMonitor& rhs ) = delete;
	cMonitor( cMonitor&& rhs ) = default;

	cMonitor& operator=( const cMonitor& rhs ) = delete;
	cMonitor& operator=( cMonitor&& rhs ) = default;

	BOOL TryEnter();

	void Enter();
	void Leave();

private:
	CRITICAL_SECTION m_csSyncObject;
};
