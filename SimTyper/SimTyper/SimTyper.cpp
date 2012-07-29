// SimTyper.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Shlwapi.h"
#include "Shellapi.h"
#include "SimTyper.h"
#include <mmsystem.h>

#define MAX_LOADSTRING	100
#define WM_TASKMANAGE	WM_USER+1

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HWND g_wnd;

// keyboard hook
HHOOK g_keyhook;

// low level keyboard proc
LRESULT CALLBACK LowLevelKeyboardProc(int,WPARAM,LPARAM);

// when key down , play the se
void onKeyDown( DWORD keyVal );

// setup notify icon
void setupNotifyIcon( bool isRemove = false );


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SIMTYPER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, SW_HIDE))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIMTYPER));

	// setup notify icon
	setupNotifyIcon();

	// install the keyboard hook
	g_keyhook = SetWindowsHookEx(WH_KEYBOARD_LL,LowLevelKeyboardProc,hInstance,NULL);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE( ICON_CY64 ));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+3);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE( ICON_CY48 ));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowEx( WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_OVERLAPPED|WS_CAPTION|WS_THICKFRAME|WS_MINIMIZEBOX|WS_SYSMENU,
      CW_USEDEFAULT, CW_USEDEFAULT, 480, 320, NULL, NULL, hInstance, NULL );

   if (!hWnd)
   {
      return FALSE;
   }
   
   g_wnd = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	HMENU menu;
	HMENU subMenu;
	POINT pt;
	RECT rect;

	switch (message)
	{
	case WM_COMMAND:
		if( LOWORD(wParam) == IDM_EXIT )
		{
			setupNotifyIcon( true );
			PostQuitMessage(0);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TASKMANAGE:
		if( lParam == WM_RBUTTONDOWN )
		{
			GetCursorPos( &pt );
			menu = LoadMenu( hInst, MAKEINTRESOURCE( MENU_TASK_BAR ) );
			subMenu = GetSubMenu( menu, 0 );

			TrackPopupMenu( subMenu,TPM_LEFTALIGN|TPM_HORIZONTAL|TPM_VERTICAL,pt.x,pt.y, NULL, g_wnd, &rect );
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


// global keyboard hook
LRESULT CALLBACK LowLevelKeyboardProc(int nCode,WPARAM wParam,LPARAM lParam)
{
 	KBDLLHOOKSTRUCT* keyInfo = (KBDLLHOOKSTRUCT*)lParam;

 	if( wParam==WM_KEYDOWN )
 	{
		DWORD key = keyInfo->vkCode;

 	 	onKeyDown( key );
 	}

	return CallNextHookEx( g_keyhook, nCode, wParam, lParam );
}


void setupNotifyIcon( bool isRemove )
{
	NOTIFYICONDATA nDat;

	nDat.cbSize = sizeof( NOTIFYICONDATA );
	nDat.hWnd = g_wnd;
	nDat.uID = 555;
	nDat.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	nDat.uCallbackMessage = WM_TASKMANAGE;
	nDat.hIcon = LoadIcon( hInst, MAKEINTRESOURCE( ICON_CY32 ) );
	strcpy( nDat.szTip, "Click to close" );
	strcpy( nDat.szInfo, "Click to close" );
	nDat.uVersion = NOTIFYICON_VERSION;

	if( isRemove )
	{
		Shell_NotifyIcon( NIM_DELETE, &nDat );
	}
	else
	{
		Shell_NotifyIcon( NIM_ADD, &nDat );
	}
}


// play the se
void onKeyDown( DWORD keyVal )
{
	int sound = SND_KEY_NEW01;

	if( keyVal == VK_RETURN )
	{
		sound = SND_RETURN;
	}

	if( keyVal == VK_SPACE )
	{
		sound = SND_SPACE_NEW;
	}

	if( keyVal == VK_BACK )
	{
		sound = SND_BACKSPACE;
	}

	if( keyVal >= 0x30 && keyVal <= 0x39 )
	{
		sound = SND_KEY_NEW01;
	}

	if( keyVal >= 0x41 && keyVal <= 0x4a )
	{
		sound = SND_KEY_NEW02;
	}

	if( keyVal > 0x4a && keyVal <= 0x53 )
	{
		sound = SND_KEY_NEW03;
	}

	if( keyVal > 0x53 && keyVal <= 0x5a )
	{
		sound = SND_KEY_NEW04;
	}

	PlaySound( (LPCSTR)sound, hInst, SND_RESOURCE|SND_ASYNC );
}
