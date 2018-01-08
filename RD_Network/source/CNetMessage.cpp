
#include	"CNetMessage.h"


CNetMessage::CNetMessage( void ) : m_type( 0 ), m_addDataLen( 0 ), m_pAddData( NULL )
{
}


CNetMessage::~CNetMessage( void )
{
	if( m_pAddData )
	{
		delete	[] m_pAddData;
	}
}


CNetMessage::CNetMessage( const CNetMessage & src ) : m_type( src.m_type ), m_addDataLen( src.m_addDataLen ), m_pAddData( NULL )
{
	if( src.m_pAddData )
	{
		m_pAddData = new char[ src.m_addDataLen ];
		if( m_pAddData )
		{
			memcpy( m_pAddData, src.m_pAddData, sizeof( char ) * src.m_addDataLen );
		}
		else
		{
			m_pAddData = NULL;
			m_addDataLen = 0;
		}
	}
}


CNetMessage & CNetMessage::operator=( const CNetMessage & src )
{
	if( this != (&src) )
	{
		m_type = src.m_type;
		m_addDataLen = src.m_addDataLen;

		if( memcmp( m_pAddData, src.m_pAddData, sizeof( char ) * src.m_addDataLen ) != 0 )
		{
			if( m_pAddData )
			{
				delete	[] m_pAddData;
				m_pAddData = NULL;
			}

			if( src.m_pAddData )
			{
				m_pAddData = new char [ src.m_addDataLen ];
				if( m_pAddData )
				{
					memcpy( m_pAddData, src.m_pAddData, sizeof( char ) * src.m_addDataLen );
				}
				else
				{
					m_pAddData = NULL;
					m_addDataLen = 0;
				}
			}
		}
	}

	return (*this);
}


const unsigned int CNetMessage::GetType( void ) const
{
	return m_type;
}


const unsigned int CNetMessage::GetAddDataLen( void ) const
{
	return m_addDataLen;
}


const char * const CNetMessage::GetpAddData( void ) const
{
	return m_pAddData;
}


void CNetMessage::SetType( const unsigned int aType )
{
	m_type = aType;
}


int CNetMessage::SendData( SOCKET socket, int aFlags, unsigned int aType, unsigned int aAddDataLen, const char * const apAddData )
{
	int	errorCode;
	int forwardReturnValue;
	int backwardReturnValue;

	// Set data member
	m_type = aType;
	m_addDataLen = aAddDataLen;

	if( m_pAddData )
	{
		delete	[] m_pAddData;
		m_pAddData = NULL;
	}

	m_pAddData = new char[ m_addDataLen ];
	if( m_pAddData )
	{
		memcpy( m_pAddData, apAddData, sizeof( char ) * m_addDataLen );
	}
	else
	{
		m_pAddData = NULL;
		m_addDataLen = 0;
	}

	// Send application protocol header
	forwardReturnValue = send( socket, (char*)this, sizeof( *this ), aFlags );
	if( forwardReturnValue == SOCKET_ERROR )
	{
		errorCode = WSAGetLastError();
		return ( forwardReturnValue );
	}

	// Send application protocol addtional data
	backwardReturnValue = send( socket, (char*)m_pAddData, m_addDataLen, aFlags );
	if( backwardReturnValue == SOCKET_ERROR )
	{
		errorCode = WSAGetLastError();
	}
	else
	{
		backwardReturnValue += forwardReturnValue;
	}

	return ( backwardReturnValue );


	//char * tempBuffer = new char[ sizeof( CNetMessage ) + (sizeof( char ) * m_addDataLen) ];

	//memcpy( tempBuffer, this, sizeof( CNetMessage ) );
	//memcpy( tempBuffer + sizeof( CNetMessage ), m_pAddData, sizeof( char ) * m_addDataLen );

	//tempReturnValue = send( socket, tempBuffer, sizeof( CNetMessage ) + (sizeof( char ) * m_addDataLen), aFlags );

	//if( tempBuffer )
	//{
	//	delete	[] tempBuffer;
	//}
}


int CNetMessage::ReceiveData( SOCKET socket, int aFlags )
{
	if( m_pAddData )
	{
		delete	[] m_pAddData;
		m_pAddData = NULL;
	}


	int errorCode;
	int forwardReturnValue;
	int backwardReturnValue;

	// Receive application protocol header
	forwardReturnValue = RD_Network::recvn( socket, (char*)this, sizeof( *this ), aFlags );
	if( forwardReturnValue == SOCKET_ERROR )
	{
		errorCode = WSAGetLastError();
		return ( forwardReturnValue );
	}

	m_pAddData = NULL;
	m_pAddData = new char[ m_addDataLen ];
	if( m_pAddData )
	{
		backwardReturnValue = RD_Network::recvn( socket, (char*)m_pAddData, m_addDataLen, aFlags );
	}
	else
	{
		m_pAddData = NULL;
		m_addDataLen = 0;
		backwardReturnValue = 0;
	}

	if( backwardReturnValue == SOCKET_ERROR )
	{
		errorCode = WSAGetLastError();
	}
	else
	{
		backwardReturnValue += forwardReturnValue;
	}

	return backwardReturnValue;
}