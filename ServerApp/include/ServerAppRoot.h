
#ifndef		__SERVERAPPROOT_H_92mxm_vmd9w_82kd9__
#define		__SERVERAPPROOT_H_92mxm_vmd9w_82kd9__



#include	"wspstr.h"
#include	"cMonitor.h"
#include	"cLog.h"
#include	"cSingleton.h"
#include	"cIOCP.h"
#include	"ErrorHandler.h"
#include	"CMiniDump.h"

#include	"MatchlessProtocol.h"
#include	"MatchlessCClient.h"
#include	"MatchlessCSkill.h"
#include	"MatchlessCState.h"

#include	"MatchlessServerCTimer.h"
#include	"MatchlessServerCClient.h"



#define		BUFSIZE		512


class cConnection;


namespace	MatchlessServer
{
	struct SkillMessageInfo
	{
		unsigned int	m_Caster;
		unsigned int	m_Target;
		unsigned int	m_SkillKind;
	};
}



extern MatchlessServer::CTimer							g_Timer;

extern std::map< SOCKET, unsigned int >				g_mClientID;				// < SOCKET, Client ID >
extern cMonitor										g_csClientID;

extern std::list< unsigned int >						g_ReuseClientIDlist;
extern unsigned int										g_LargestClientID;
extern std::map< unsigned int, std::shared_ptr< MatchlessServer::CClient > >	g_ClientList;			// < ID, SOCKET >
extern cMonitor											g_ClientListMonitor;
extern std::map< unsigned short int, int >				g_TeamPlayerNumMap;
extern bool												g_IsGameStartable;
extern bool												g_IsAcceptable;
extern unsigned int										g_CurrentMapKind;

extern std::map< unsigned int, MatchlessServer::SkillMessageInfo >		g_SkillMessageList;		// < Tick, SkillInfo >



DWORD WINAPI TimerThread( LPVOID arg );
DWORD WINAPI GameProcessThread( LPVOID arg );

unsigned int GetClientID( void );
int ReturnClientID( const unsigned int aID );

int OutputServerInitialInfo( const SOCKADDR_IN & aAddrInfo, const SOCKET aListenSocket );
bool DoNeedRoomMaster( void );
int ChangeTeamPlayerNum( const unsigned short int aBefore, const unsigned short int aAfter );
//bool IsGameStartable( void );
bool IsGameFinish( void );
bool HandleSkillRequest( const bool aIsCastStart, const Matchless::ECharacterSkill aSkillKind, MatchlessServer::CClient& aCaster, MatchlessServer::CClient& aTarget );
bool IsNowCasting( const unsigned int aID, const bool aIsCancel );

bool PrintPacket( const CNetMessage & msg );


DWORD WINAPI ProcessClient_Accept( const cConnection& connection );
DWORD WINAPI ProcessClient_Recv( const cConnection& connection, cIPacket& iPacket );


int InitLog();
int CloseLog();


#endif		// __SERVERAPPROOT_H_92mxm_vmd9w_82kd9__
