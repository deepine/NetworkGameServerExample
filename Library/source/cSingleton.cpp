
#include "stdafx.h"
#include "cSingleton.h"


cSingleton::SingletonList cSingleton::m_listSingleton;


cSingleton::cSingleton()
{
	m_listSingleton.push_back( this );
}

cSingleton::~cSingleton()
{
	auto singleton_it = m_listSingleton.begin();
	while ( singleton_it != m_listSingleton.end() )
	{
		if ( ( *singleton_it ) == this )
			break;
		++singleton_it;
	}

	m_listSingleton.erase( singleton_it );
}

void cSingleton::releaseAll()
{
	auto singleton_rit = m_listSingleton.rbegin();
	while ( singleton_rit != m_listSingleton.rend() )
	{
		(*singleton_rit)->releaseInstance();
		++singleton_rit;
	}

	m_listSingleton.clear();
}
