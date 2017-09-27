#include <windows.h>
#include "globals.h"
#include "resource.h"
#include "server.h"

#define WM_TASKBAR_NOTIFY WM_APP + 1

/* Global variables */
static ATOM    mainWindowClass;
static LPCTSTR mainWindowClassName = TEXT("Kaia.HttpNotify.MainWindow");
static LPTSTR  gAppTitle;
static HMENU   ghMenu;
static NOTIFYICONDATA nid;

Globals globals;

/* Window functions */
static HWND CreateMainWindow(HINSTANCE hInstance, HICON hIcon);
static void CreateMainWindowClass(HINSTANCE hInstance, HICON hIcon);
static LRESULT APIENTRY MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void DoShowMenu(HWND hWnd, int x, int y);
static void DoAboutApp(HWND hWnd);
static void DoCopyData(HWND hWnd, COPYDATASTRUCT *data);


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
	
	if (hWndMain == NULL)
	{
		return 1;
	}
	
	globals.hInstance = hInstance;
	globals.hWndMain = hWndMain;
	globals.bIsRunning = TRUE;
	globals.nServerPort = 54841; /* TODO: Allow port to be set on the command line */
	
	/* Initialise notification area icon */
	nid.cbSize           = sizeof(nid);
	nid.hWnd             = hWndMain;
	nid.uID              = 1;
	nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TASKBAR_NOTIFY;
	nid.hIcon            = hIcon;
	lstrcpyn(nid.szTip, LoadStringFromResource(hInstance, IDS_TASKBAR_TOOLTIP), 63);
	Shell_NotifyIcon(NIM_ADD, &nid);
	
	/* Create server thread to listen for HTTP connections */
	HANDLE hThread = CreateThread(
		NULL,
		0,
		HandleHttpConnections,
		&globals,
		0,
		NULL
	);
	DWORD dwThreadStatus;
	if (!GetExitCodeThread(hThread, &dwThreadStatus) || dwThreadStatus != STILL_ACTIVE)
	{
		DisplayErrorMessage(hInstance, hWndMain, IDS_ERROR_THREAD);
		DestroyWindow(hWndMain);
		return 1;
	}
		
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
	Shell_NotifyIcon(NIM_DELETE, &nid);
	
	globals.bIsRunning = FALSE;
	WaitForSingleObject(hThread, INFINITE);
	
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
	do
	{
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
		case WM_COPYDATA:
			DoCopyData(hwnd, (COPYDATASTRUCT*)lParam);
			return TRUE;
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


static void DoCopyData(HWND hWnd, COPYDATASTRUCT *data)
{

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
