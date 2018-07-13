// DBServerApp.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"

#include "LibraryDef.h"
#include "cException.h"
#include "cLog.h"
#include "CMiniDump.h"


int InitLog()
{
	sLogConfig LogConfig;
	LogConfig.s_eLogFileType = eLogFileType::FILETYPE_TEXT;
	//LogConfig.s_hWnd = hWnd;
	LogConfig.s_sLogFileName = _T( "DBServerApp" );

	//LogConfig.s_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_OUTPUTWND ) ] = eLogInfoType::LOG_ALL;
	LogConfig.s_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_FILE ) ] = eLogInfoType::LOG_ALL;

	if ( !INIT_LOG( LogConfig ) )
	{
		return 1;
	}

	return RETURN_SUCCEED;
}

int CloseLog()
{
	CLOSE_LOG();

	return RETURN_SUCCEED;
}


int Startup()
{
	CMiniDump::Begin();

	if ( InitLog() )
	{
		throw cException_FailedToCreateObject{ "Log", __FILE__, __LINE__ };
	}


	SQLHENV		henv {};
	SQLHDBC		hdbc {};
	SQLHSTMT	hstmt = 0;

	// Allocate environment handle
	SQLRETURN ret = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv );
	// Set the ODBC version environment attribute
	if ( SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret )
	{
		ret = SQLSetEnvAttr( henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0 );
		// Allocate connection handle
		if ( SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret )
		{
			ret = SQLAllocHandle( SQL_HANDLE_DBC, henv, &hdbc );
			// Set login timeout to 5 seconds
			if ( SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret )
			{
				SQLSetConnectAttr( hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0 );

				// Connect to data source
				tstring		sDNS { _T( "MatchlessGame" ) };
				tstring		sUser { _T( "sa" ) };
				tstring		sAuth { _T( "root" ) };
				ret = SQLConnect( hdbc, (SQLTCHAR*)sDNS.c_str(), static_cast<SQLSMALLINT>( sDNS.length() ),
					(SQLTCHAR*)sUser.c_str(), static_cast<SQLSMALLINT>( sUser.length() ), (SQLTCHAR*)sAuth.c_str(), static_cast<SQLSMALLINT>( sAuth.length() ) );
				// Allocate statement handle
				if ( SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret )
				{
					ret = SQLAllocHandle( SQL_HANDLE_STMT, hdbc, &hstmt );
				}
			}
		}
	}

	return RETURN_SUCCEED;
}

int Run()
{
	//tstring sSQLCmd{};
	//sSQLCmd = _T( "INSERT INTO [dbo].[LogCommon] ([Time],[Type],[Log]) VALUES (GETDATE(), 0, 'For Test~!')" );
	//ret = SQLExecDirect( hstmt, (SQLTCHAR*)sSQLCmd.c_str(), SQL_NTS );
	//SQLCancel( hstmt );

	return RETURN_SUCCEED;
}

int Cleanup()
{
	//if ( hstmt )
	//{
	//	SQLFreeHandle( SQL_HANDLE_STMT, hstmt );
	//}

	//if ( hdbc )
	//{
	//	SQLDisconnect( hdbc );
	//	SQLFreeHandle( SQL_HANDLE_DBC, hdbc );
	//}

	//if ( henv )
	//{
	//	SQLFreeHandle( SQL_HANDLE_ENV, henv );
	//}

	CloseLog();

	CMiniDump::End();

	return RETURN_SUCCEED;
}


int main()
try {
	if ( Startup() )
	{
		WriteLog( _T( "Failed DBServerApp Startup()" ), { eLogInfoType::LOG_ERROR_HIGH } );
		return 1;
	}

	if ( Run() )
	{
		WriteLog( _T( "Occur Error while DBServerApp Run()" ), { eLogInfoType::LOG_ERROR_HIGH } );
		return 10000;
	}

	Cleanup();

    return RETURN_SUCCEED;
}
catch ( const cException& e ) {
	tstring notice;

	notice = wsp::to( e.what() );

	WriteLog( notice, { eLogInfoType::LOG_ERROR_HIGH } );

	return 0xa0000003;
}
catch ( const std::runtime_error& e ) {
	tstring notice;

	notice = wsp::to( e.what() );

	WriteLog( notice, { eLogInfoType::LOG_ERROR_HIGH } );

	return 0xa0000002;
}
catch ( const std::exception& e ) {
	tstring notice;

	notice = wsp::to( e.what() );

	WriteLog( notice, { eLogInfoType::LOG_ERROR_HIGH } );

	return 0xa0000001;
}
catch ( ... ) {
	WriteLog( _T( "Unknown Exception" ), { eLogInfoType::LOG_ERROR_HIGH } );

	return 0xa0000000;
}
