#pragma once


#include	"linkopt.h"


class NETLIB_API cVBuffer
{
public:
	static const int MAX_VBUFFER_SIZE = 4 * 1024;
	static const int MAX_PBUFSIZE = MAX_VBUFFER_SIZE;

public:
	cVBuffer( int nMaxBufSize = MAX_VBUFFER_SIZE );
	~cVBuffer();

	cVBuffer( const cVBuffer& rhs ) = delete;
	cVBuffer& operator=( const cVBuffer& rhs ) = delete;

	void GetChar( char& cCh );
	void GetShort( short& sNum );
	void GetInteger( int& nNum );
	void GetString( char* pszBuffer );
	void GetStream( char* pszBuffer, const short sLen );
	void SetInteger( const int nI );
	void SetShort( const short sShort );
	void SetChar( const char cCh );
	void SetString( const char* pszBuffer );
	void SetStream( const char* pszBuffer, const short sLen );

	void SetBuffer( char* pVBuffer );

	inline int GetMaxBufSize() { return m_nMaxBufSize; }
	inline int GetCurBufSize() { return m_nCurBufSize; }
	inline char* GetCurMark() { return m_pCurMark; }
	inline char* GetBeginMark() { return m_pszVBuffer; }

	bool CopyBuffer( char* pDestBuffer );
	void Init();

private:
	char* m_pszVBuffer;
	char* m_pCurMark;
	int m_nMaxBufSize;
	int m_nCurBufSize;
};
