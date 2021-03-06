
#include	"stdafx.h"
#include	"cVBuffer.h"
#include	"LibraryDef.h"


cVBuffer::cVBuffer( int nMaxBufSize )
{
	m_pszVBuffer = new char[ nMaxBufSize ];
	m_nMaxBufSize = nMaxBufSize;
	Init();
}

cVBuffer::~cVBuffer()
{
	if ( nullptr != m_pszVBuffer )
		delete[] m_pszVBuffer;
}

void cVBuffer::Init()
{
	m_pCurMark = m_pszVBuffer + PACKET_SIZE_LENGTH;
	m_nCurBufSize = PACKET_SIZE_LENGTH;
}

void cVBuffer::GetChar( char& cCh )
{
	cCh = (unsigned char)*m_pCurMark;
	m_pCurMark += 1;
	m_nCurBufSize += 1;
}

void cVBuffer::GetShort( short& sNum )
{
	sNum = (unsigned char)*m_pCurMark +
		( ( (unsigned char)*( m_pCurMark + 1 ) ) << 8 );
	m_pCurMark += 2;
	m_nCurBufSize += 2;
}

void cVBuffer::GetInteger( int& nNum )
{
	nNum = (unsigned char)m_pCurMark[ 0 ] +
		( (unsigned char)m_pCurMark[ 1 ] << 8 ) +
		( (unsigned char)m_pCurMark[ 2 ] << 16 ) +
		( (unsigned char)m_pCurMark[ 3 ] << 24 );
	m_pCurMark += 4;
	m_nCurBufSize += 4;
}

void cVBuffer::GetStream( char* pszBuffer, const short sLen )
{
	if ( sLen < 0 || sLen > MAX_PBUFSIZE )
		return;
	CopyMemory( pszBuffer, m_pCurMark, sLen );
	m_pCurMark += sLen;
	m_nCurBufSize += sLen;
}

void cVBuffer::GetString( char* pszBuffer )
{
	short sLength;
	GetShort( sLength );
	if ( sLength < 0 || sLength > MAX_PBUFSIZE )
		return;
	CopyMemory( pszBuffer, m_pCurMark, sLength );
	*( pszBuffer + sLength ) = NULL;
	m_pCurMark += sLength;
	m_nCurBufSize += sLength;
}

void cVBuffer::SetInteger( const int nI )
{
	for ( size_t i = 0 ; i < sizeof( nI ) ; ++i )
	{
		*m_pCurMark++ = nI >> ( i * 8 );
	}

	m_nCurBufSize += sizeof( nI );
}

void cVBuffer::SetShort( const short sShort )
{
	for ( size_t i = 0 ; i < sizeof( sShort ) ; ++i )
	{
		*m_pCurMark++ = sShort >> ( i * 8 );
	}

	m_nCurBufSize += sizeof( sShort );
}

void cVBuffer::SetChar( const char cCh )
{
	for ( size_t i = 0 ; i < sizeof( cCh ) ; ++i )
	{
		*m_pCurMark++ = cCh >> ( i * 8 );
	}

	m_nCurBufSize += sizeof( cCh );
}

void cVBuffer::SetStream( const char* pszBuffer, const short sLen )
{
	CopyMemory( m_pCurMark, pszBuffer, sLen );
	m_pCurMark += sLen;
	m_nCurBufSize += sLen;
}

void cVBuffer::SetString( const char* pszBuffer )
{
	int sLen = static_cast<int>( strlen( pszBuffer ) );
	if ( sLen < 0 || sLen > MAX_PBUFSIZE )
		return;
	SetShort( sLen );

	CopyMemory( m_pCurMark, pszBuffer, sLen );
	m_pCurMark += sLen;
	m_nCurBufSize += sLen;
}

bool cVBuffer::CopyBuffer( char* pDestBuffer )
{
	CopyMemory( m_pszVBuffer, reinterpret_cast<char*>( &m_nCurBufSize ), PACKET_SIZE_LENGTH );
	CopyMemory( pDestBuffer, m_pszVBuffer, m_nCurBufSize );
	return true;
}

void cVBuffer::SetBuffer( char* pVBuffer )
{
	m_pCurMark = pVBuffer;
	m_nCurBufSize = 0;
}
