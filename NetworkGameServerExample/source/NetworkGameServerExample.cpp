// NetworkGameServerExample.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "NetworkGameServerExample.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NETWORKGAMESERVEREXAMPLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 응용 프로그램 초기화를 수행합니다.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NETWORKGAMESERVEREXAMPLE));

    MSG msg;

    // 기본 메시지 루프입니다.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NETWORKGAMESERVEREXAMPLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_NETWORKGAMESERVEREXAMPLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		{
			cSyncClass sc;
			sc.IncrementInteger();
		}
		{
			cTickThread TickThread;
			TickThread.CreateThread( 1000 );
			TickThread.Run();
			Sleep( 10000 );
			TickThread.DestroyThread();
		}
		cSingleton< cVBuffer >::Get()->Init();
		{
			cQueue< int > Queue;
			for ( int i = 0 ; i < 100 ; ++i ) {
				Queue.PushQueue( i );
			}
			while ( !Queue.IsEmptyQueue() )
			{
				std::cout << Queue.GetFrontQueue() << "\n";
				Queue.PopQueue();
			}
		}
		{
			sLogConfig LogConfig;
			LogConfig.s_eLogFileType = eLogFileType::FILETYPE_TEXT;
			LogConfig.s_hWnd = hWnd;
			LogConfig.s_sLogFileName = _T( "예제" );

			LogConfig.s_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_OUTPUTWND ) ] = eLogInfoType::LOG_ALL;
			LogConfig.s_vLogInfoTypes[ static_cast<size_t>( eLogStorageType::STORAGE_FILE ) ] = eLogInfoType::LOG_INFO_NORMAL | eLogInfoType::LOG_ERROR_CRITICAL;

			INIT_LOG( LogConfig );

			LOG( eLogInfoType::LOG_ERROR_CRITICAL, { tstring( _T( "치명적인 에러 발생!!!" ) ) } );
			LOG( eLogInfoType::LOG_INFO_LOW, { tstring( _T( "낮은 수준의 정보 알림~" ) ) } );
			LOG( eLogInfoType::LOG_INFO_NORMAL, { tstring( _T( "중간 수준의 정보 알림~~" ) ) } );
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다.
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.
			std::wostringstream oss;
			oss << ( cSingleton< cUtilManager >::Get()->CalcSum( 1230, 5670 ) );
			TextOut( hdc, 20, 20, oss.str().c_str(), oss.str().length() );

			int nPacketLen = 1024;
			char szPacket[ 1024 ];
			cSingleton< cVBuffer >::Get()->SetShort( 12345 );
			cSingleton< cVBuffer >::Get()->SetInteger( 123456789 );
			cSingleton< cVBuffer >::Get()->SetChar( 'M' );
			cSingleton< cVBuffer >::Get()->SetString( "ABC" );
			cSingleton< cVBuffer >::Get()->CopyBuffer( szPacket );

			short nSN;
			int nKey = 0;
			char cSex;
			char szDesc[ 10 ];

			cSingleton< cVBuffer >::Get()->SetBuffer( szPacket );
			cSingleton< cVBuffer >::Get()->GetInteger( nPacketLen );
			cSingleton< cVBuffer >::Get()->GetShort( nSN );
			cSingleton< cVBuffer >::Get()->GetInteger( nKey );
			cSingleton< cVBuffer >::Get()->GetChar( cSex );
			cSingleton< cVBuffer >::Get()->GetString( szDesc );

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
		cSingleton< cUtilManager >::releaseInstance();
		CLOSE_LOG();
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}



cSyncClass::cSyncClass()
{
	m_nInteger = 0;
}

cSyncClass::~cSyncClass()
{
	;
}

void cSyncClass::IncrementInteger()
{
	cMonitor::Owner lock( m_csInteger );
	m_nInteger++;
}



int cUtilManager::CalcSum( int a, int b )
{
	return a + b;
}



void cTickThread::OnProcess()
{
	std::cout << "현재 틱 횟수 : " << m_dwTickCount << "\n";
}
