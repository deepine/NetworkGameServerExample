
#ifndef		__MATCHLESSCCLIENT_H_9v9e2_vj92f_vj923__
#define		__MATCHLESSCCLIENT_H_9v9e2_vj92f_vj923__



#include		"CNetClient.h"
#include		"MatchlessCPlayerInfo.h"



namespace	Matchless
{
	class CClient
	{

	public:

		CNetClient		m_NetSystem;
		CPlayerInfo		m_PlayerInfo;

	};

	void Encode( cOPacket& oPacket, const CClient& client );
	void Decode( cIPacket& iPacket, CClient& client );
}



#endif		// __MATCHLESSCCLIENT_H_9v9e2_vj92f_vj923__
