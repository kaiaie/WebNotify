#ifndef GLOBALS_H
#define GLOBALS_H

typedef struct globals
{
	HINSTANCE       hInstance;
	HWND            hWndMain;
	BOOL            bIsRunning;
	int             nServerPort;
	NOTIFYICONDATA  nid;
} Globals;

#endif /* GLOBALS_H */
