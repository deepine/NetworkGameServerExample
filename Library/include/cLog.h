#pragma once


#include	<queue>
#include	<memory>

#include	"linkopt.h"
#include	"cMonitor.h"
#include	"cThread.h"


enum class eLogInfoType
{
	LOG_NONE				= 0x00000000,
	LOG_INFO_LOW			= 0x00000001,
	LOG_INFO_NORMAL			= 0x00000002,
	LOG_INFO_HIGH			= 0x00000004,
	LOG_INFO_CRITICAL		= 0x00000008,
	LOG_INFO_ALL			= ( LOG_INFO_LOW | LOG_INFO_NORMAL | LOG_INFO_HIGH | LOG_INFO_CRITICAL ),
	LOG_ERROR_LOW			= 0x00000010,
	LOG_ERROR_NORMAL		= 0x00000020,
	LOG_ERROR_HIGH			= 0x00000040,
	LOG_ERROR_CRITICAL		= 0x00000080,
	LOG_ERROR_ALL			= ( LOG_ERROR_LOW | LOG_ERROR_NORMAL | LOG_ERROR_HIGH | LOG_ERROR_CRITICAL ),
	LOG_ALL					= ( LOG_INFO_ALL | LOG_ERROR_ALL ),
};

constexpr eLogInfoType operator&( const eLogInfoType e1, const eLogInfoType e2 )
{
	return static_cast<eLogInfoType>( static_cast<unsigned int>( e1 ) & static_cast<unsigned int>( e2 ) );
}

constexpr eLogInfoType operator|( const eLogInfoType e1, const eLogInfoType e2 )
{
	return static_cast<eLogInfoType>( static_cast<unsigned int>( e1 ) | static_cast<unsigned int>( e2 ) );
}

tstring GetLogInfoTypeString( const eLogInfoType e );

enum class eLogStorageType
{
	STORAGE_FILE			= 0x00000000,
	STORAGE_DB				= 0x00000001,
	STORAGE_WINDOW			= 0x00000002,
	STORAGE_OUTPUTWND		= 0x00000003,
	STORAGE_UDP				= 0x00000004,
	STORAGE_TCP				= 0x00000005,

	STORAGE_MAX
};

enum class eLogFileType
{
	FILETYPE_NONE			= 0x00000000,
	FILETYPE_TEXT			= 0x00000001,
};

struct sLogMsg
{
	eLogInfoType	s_eLogInfoType;
	tstring			s_sOutputString;
};

struct sLogConfig
{
	static const DWORD	DEFAULT_TICK = 200;
	static const int	DEFAULT_UDPPORT = 19999;
	static const int	DEFAULT_TCPPORT = 19998;
	static const size_t	MAX_STORAGE_TYPE = static_cast<size_t>( eLogStorageType::STORAGE_MAX );

	std::vector< eLogInfoType >	s_vLogInfoTypes;
	tstring				s_sLogFileName;

	eLogFileType		s_eLogFileType;

	tstring		s_sIP;
	int			s_nUDPPort;
	int			s_nTCPPort;
	int			s_nServerType;

	tstring		s_sDSNNAME;
	tstring		s_sDSNID;
	tstring		s_sDSNPW;

	HWND		s_hWnd;

	DWORD		s_dwProcessTick;

	DWORD		s_dwFileMaxSize;

	sLogConfig()
		: s_dwProcessTick( DEFAULT_TICK )
		, s_nUDPPort( DEFAULT_UDPPORT )
		, s_nTCPPort( DEFAULT_TCPPORT )
		, s_dwFileMaxSize( 1024 * 50000 )
		, s_vLogInfoTypes( MAX_STORAGE_TYPE, eLogInfoType::LOG_NONE )
	{
	}
};

class NETLIB_API cLog : public cThread
{
public:
	static const DWORD MAX_LOGFILE_SIZE = 100 * 1024 * 1024;
public:
	//cLog();
	//~cLog();

	bool Init( sLogConfig& LogConfig );
	void LogOutput( eLogInfoType eLogInfo, const tstring& OutputString );
	void LogOutputLastErrorToMsgBox( const tstring& sOutputString );
	void CloseAllLog();

	virtual void OnProcess();
	void SetHWND( HWND hWnd = NULL ) { m_hWnd = hWnd; }

	size_t GetQueueSize() const { return m_queueLogMsg.size(); }

	void InsertMsgToQueue( sLogMsg* pLogMsg )
	{
		m_queueLogMsg.push( pLogMsg );
	}

private:
	std::vector< eLogInfoType >	m_vLogInfoTypes;
	tstring				m_sLogFileName;
	eLogFileType		m_eLogFileType;

	tstring		m_sIP;
	int			m_nUDPPort;
	int			m_nTCPPort;
	int			m_nServerType;

	tstring		m_sDSNNAME;
	tstring		m_sDSNID;
	tstring		m_sDSNPW;

	tstring		m_sOutStr;
	HWND		m_hWnd;

	HANDLE		m_hLogFile;

	SOCKET		m_sockUdp;
	SOCKET		m_sockTcp;

	std::queue< sLogMsg* >	m_queueLogMsg;
	int			m_nMsgBufferIdx;
	DWORD		m_dwFileMaxSize;

	void OutputFile( const tstring& sOutputString );
	void OutputDB( const tstring& sOutputString );
	void OutputWindow( eLogInfoType eLogInfo, const tstring& sOutputString );
	void OutputOutputWnd( const tstring& sOutputString );
	void OutputUDP( eLogInfoType eLogInfo, const tstring& sOutputString );
	void OutputTCP( eLogInfoType eLogInfo, const tstring& sOutputString );

	bool InitDB();
	bool InitFile();
	bool InitUDP();
	bool InitTCP();
};

const size_t MAX_QUEUE_CNT = 10 * 10000;

static tstring g_sOutStr;
static std::vector< sLogMsg > g_vLogMsg;
static cMonitor g_csLog;

bool NETLIB_API INIT_LOG( sLogConfig& LogConfig );

void NETLIB_API LOG( eLogInfoType eLogInfoType, const tstring& s );
void NETLIB_API LOG( eLogInfoType eLogInfoType, const std::initializer_list<tstring>& sl );

void NETLIB_API LOG_LASTERROR( const std::initializer_list<tstring>& sl );

void NETLIB_API CLOSE_LOG();
