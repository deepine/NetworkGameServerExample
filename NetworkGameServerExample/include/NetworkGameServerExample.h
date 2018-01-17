#pragma once

#include "resource.h"

#include "cMonitor.h"
#include "cSingleton.h"


class cSyncClass : public cMonitor
{
public:
	cSyncClass();
	~cSyncClass();

	void IncrementInteger();

private:
	int			m_nInteger;
	cMonitor	m_csInteger;
};


class cUtilManager : public cSingleton
{
	DECLEAR_SINGLETON( cUtilManager );

public:
	int CalcSum( int a, int b );

};

CREATE_FUNCTION( cUtilManager, UtilManager )
