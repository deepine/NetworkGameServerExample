#pragma once


#include <list>

#include "linkopt.h"


#define DECLEAR_SINGLETON( className ) \
public: \
	static className* Instance(); \
	virtual void releaseInstance(); \
private: \
	static className* m_pInstance;

#define CREATE_FUNCTION( className, funcName ) \
	static className* ##funcName() \
	{ \
		return className::Instance(); \
	}

#define IMPLEMENT_SINGLETON( className ) \
	className* className::m_pInstance = NULL; \
	className* className::Instance() \
	{ \
		if ( NULL == m_pInstance ) \
		{ \
			m_pInstance = new className; \
		} \
		return m_pInstance; \
	} \
	void className::releaseInstance() \
	{ \
		if ( NULL != m_pInstance ) \
		{ \
			delete m_pInstance; \
			m_pInstance = NULL; \
		} \
	}


class NETLIB_API cSingleton
{
public:
	using SingletonList = std::list< cSingleton* >;

	cSingleton();
	virtual ~cSingleton();

public:
	virtual void releaseInstance() = 0;
	static void releaseAll();

private:
	static SingletonList m_listSingleton;
};
