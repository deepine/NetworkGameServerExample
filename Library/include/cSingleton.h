#pragma once


#include <list>

#include "linkopt.h"


template< typename T >
class NETLIB_API cSingleton
{
public:
	using Ptr_type = T*;

	static Ptr_type Get()
	{
		return Instance();
	}

	static void releaseInstance()
	{
		if ( m_pInstance != nullptr )
		{
			delete m_pInstance;
			m_pInstance = nullptr;
		}
	}

private:
	static Ptr_type m_pInstance;

	static Ptr_type Instance()
	{
		if ( m_pInstance == nullptr )
		{
			m_pInstance = new T{};
		}
		return m_pInstance;
	}
};

template< typename T >
typename cSingleton<T>::Ptr_type cSingleton<T>::m_pInstance;
