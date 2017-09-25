#include <windows.h>
#include "resource.h"

#define WM_TASKBAR_NOTIFY WM_APP + 1

/* Global variables */
static ATOM mainWindowClass;
static LPCTSTR mainWindowClassName = TEXT("Kaia.HttpNotify.MainWindow");
static LPTSTR gAppTitle;
static HMENU ghMenu;

/* Window functions */
static HWND CreateMainWindow(HINSTANCE hInstance, HICON hIcon);
static void CreateMainWindowClass(HINSTANCE hInstance, HICON hIcon);
static LRESULT APIENTRY MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void DoShowMenu(HWND hWnd, int x, int y);
static void DoAboutApp(HWND hWnd);


/* Utility functions */
static LPTSTR LoadStringFromResource(HINSTANCE hInstance, UINT uID);
static void DisplayErrorMessage(HINSTANCE hInstance, HWND hWnd, UINT messageCode);

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	gAppTitle = LoadStringFromResource(hInstance, IDS_APP_TITLE);
	ghMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDM_MAIN));
	MSG msg;
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(0));
	HWND hWndMain = CreateMainWindow(hInstance, hIcon);	
	//NOTIFYICONDATA nid;
	//nid.cbSize = sizeof(nid);
	//nid.hWnd = hWndMain;
	
	/* Start message loop (keep window invisible; it's only there to get the
	** notification area messages)
	*/
	ShowWindow(hWndMain, SW_HIDE);
	UpdateWindow(hWndMain);
	while (GetMessage(&msg, (HWND)NULL, 0, 0) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return msg.wParam;	
}


static HWND
CreateMainWindow(HINSTANCE hInstance, HICON hIcon)
{
	HWND hWnd;
	
	CreateMainWindowClass(hInstance, hIcon);
	
	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		mainWindowClassName, 
		gAppTitle, 
		WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		(HWND)NULL, 
		NULL, 
		hInstance, 
		(LPVOID)NULL
	);

	return hWnd;
}


static void
CreateMainWindowClass(HINSTANCE hInstance, HICON hIcon)
{
	WNDCLASSEX wcx;
	
	if (mainWindowClass == 0) 
	{
		wcx.cbSize        = sizeof(wcx);
		wcx.style         = CS_HREDRAW | CS_VREDRAW; 
		wcx.lpfnWndProc   = MainWindowProc;
		wcx.cbClsExtra    = 0;
		wcx.cbWndExtra    = 0;
		wcx.hInstance     = hInstance;
		wcx.hIcon         = hIcon;
		wcx.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcx.lpszMenuName  = NULL;
		wcx.lpszClassName = mainWindowClassName;
		wcx.hIconSm       = NULL;
		
		mainWindowClass = RegisterClassEx(&wcx);
	}
}


static LPTSTR
LoadStringFromResource(HINSTANCE hInstance, UINT uID)
{
	LPTSTR lpBuffer     = NULL;
	LPTSTR lpTemp       = NULL;
	DWORD  dwBufferSize = 64;
	int    result       = 0;
	
	if ((lpBuffer = malloc(dwBufferSize)) == NULL)
	{
		DisplayErrorMessage(hInstance, (HWND)NULL, IDS_ERROR_OUT_OF_MEMORY);
		ExitProcess(1);
	}
	do {
		result = LoadString(hInstance, uID, lpBuffer, dwBufferSize);
		if (result == 0) 
		{ 
			/* Error */
			if (lpBuffer != NULL) {
				free(lpBuffer);
				lpBuffer = NULL;
				break;
			}
		}
		else if ((result + 1) <= dwBufferSize)
		{
			/* String fit into buffer */
			break;
		}
		/* Buffer too small: enlarge */
		dwBufferSize *= 2;
		if ((lpTemp = realloc(lpBuffer, dwBufferSize)) == NULL)
		{
				DisplayErrorMessage(hInstance, (HWND)NULL, IDS_ERROR_OUT_OF_MEMORY);
				ExitProcess(1);
		}
		lpBuffer = lpTemp;
	} while (TRUE);
	return lpBuffer;
}


static LRESULT APIENTRY
MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WORD  wCmdID;
	POINT p;
	switch (uMsg)
	{
		case WM_TASKBAR_NOTIFY:
			if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU)
			{
				GetCursorPos(&p);
				DoShowMenu(hwnd, p.x, p.y);
			}
			return 0;
		case WM_CONTEXTMENU:
			DoShowMenu(hwnd, LOWORD(lParam), HIWORD(lParam));
			return 0;
		case WM_COMMAND:
			wCmdID = LOWORD(wParam);
			if (wCmdID != IDM_EXIT)
			{
				if (wCmdID == IDM_ABOUT)
				{
					DoAboutApp(hwnd);
				}
				return 0;
			}
			/* FALL THROUGH */
		case WM_CLOSE:
			// if (bNotifyIconCreated) Shell_NotifyIcon(NIM_DELETE, &nid);
			DestroyWindow(hwnd);
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}


static void 
DoShowMenu(HWND hWnd, int x, int y)
{
	TrackPopupMenu(
		ghMenu,
		TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
		x,
		y,
		0,
		hWnd,
		NULL
	);
}


static void
DoAboutApp(HWND hWnd)
{
	MessageBox(hWnd, TEXT("About"), gAppTitle, MB_OK | MB_ICONINFORMATION);
}


static void
DisplayErrorMessage(HINSTANCE hInstance, HWND hWnd, UINT messageCode)
{
	LPTSTR lpMessage;
	
	if ((lpMessage = LoadStringFromResource(hInstance, messageCode)) != NULL)
	{
		MessageBox(hWnd, 
			lpMessage, 
			gAppTitle, 
			MB_OK | MB_ICONEXCLAMATION
		);		
	}
	free(lpMessage);
}
