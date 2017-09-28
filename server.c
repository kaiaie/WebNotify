#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "globals.h"
#include "resource.h"

#define BUF_LEN 1048576
#define writestr(fd, str) send(fd, str, strlen(str), 0)

static void   SendHelpPage(int socket);
static void   Send400Page(int socket);
static void   Send404Page(int socket);
static void   Send500Page(int socket);
static LPVOID LoadPageResource(DWORD resourceId, LPDWORD resourceSize);
static void   ShowNotification(char *buffer, PNOTIFYICONDATA nid, DWORD dwNotificationType);

DWORD WINAPI 
HandleHttpConnections(LPVOID lpParam)
{
	Globals *globals = (Globals *)lpParam;
	int serverSocket = INVALID_SOCKET;
    SOCKADDR_IN serverAddress;
    WSADATA wsaData;
    WORD wsaVersion = MAKEWORD(1, 1); 

    if (WSAStartup(wsaVersion, &wsaData) != NO_ERROR)
	{
		return 1;
    }
	/* Create socket */
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		WSACleanup();
		return 2;
    }
	/* Set non-blocking */
	u_long iMode = 1;
	if ((ioctlsocket(serverSocket, FIONBIO, &iMode)) != NO_ERROR)
	{
		closesocket(serverSocket);
		WSACleanup();
		return 3;
	}
	/* Bind socket */
    serverAddress.sin_family        = AF_INET;
    serverAddress.sin_addr.s_addr   = htonl(INADDR_ANY); /* TODO: Bind to local address only? for security */
    serverAddress.sin_port          = htons(globals->nServerPort);
    if ((bind(serverSocket, (LPSOCKADDR)&serverAddress, sizeof(serverAddress))) == SOCKET_ERROR)
	{
		closesocket(serverSocket);
		WSACleanup();
		return 4;
    }	
    /* Create connection queue */
    if ((listen(serverSocket, 5)) == SOCKET_ERROR)
	{
		closesocket(serverSocket);
		WSACleanup();
		return 5;
    }
	
	/* Server loop */
	while (globals->bIsRunning)
	{
		SOCKADDR_IN clientAddress;
		int         clientLen = sizeof(clientAddress);
		int         clientSocket = accept(serverSocket, (LPSOCKADDR)&clientAddress, &clientLen);
		if (clientSocket == INVALID_SOCKET)
		{
			Sleep(0L);
			continue;
		}
		char *buffer = malloc(BUF_LEN);
		if (recv(clientSocket, buffer, BUF_LEN, 0) > 0)
		{
			/* Decode endpoint */
			if (strncmp(buffer, "GET / ", 6) == 0)
			{
				SendHelpPage(clientSocket);
			}
			else if (strncmp(buffer, "POST /info", 10) == 0) {
				ShowNotification(buffer, &(globals->nid), NIIF_INFO);
			}
			else if (strncmp(buffer, "POST /warn", 10) == 0) {
				ShowNotification(buffer, &(globals->nid), NIIF_WARNING);
			}
			else if (strncmp(buffer, "POST /error", 11) == 0) {
				ShowNotification(buffer, &(globals->nid), NIIF_ERROR);
			}
			else
			{
				Send404Page(clientSocket);
			}
		}
		free(buffer);
		closesocket(clientSocket);
		Sleep(0L);
	}
	closesocket(serverSocket);	
	WSACleanup();	
	
	return 0;
}


static void
SendHelpPage(int socket)
{
	writestr(socket, "HTTP/1.1 200 OK\n");
	writestr(socket, "Content-Type: text/html\n\n");
	DWORD pageSize;
	char *page = (char *)LoadPageResource(IDR_HOMEPAGE, &pageSize);
	send(socket, page, pageSize, 0);
}


static void
Send400Page(int socket)
{
	writestr(socket, "HTTP/1.1 400 Bad Request\n");
	writestr(socket, "Content-Type: text/html\n\n");	
	DWORD pageSize;
	char *page = (char *)LoadPageResource(IDR_400, &pageSize);
	send(socket, page, pageSize, 0);
}


static void
Send404Page(int socket)
{
	writestr(socket, "HTTP/1.1 404 Not Found\n");
	writestr(socket, "Content-Type: text/html\n\n");	
	DWORD pageSize;
	char *page = (char *)LoadPageResource(IDR_404, &pageSize);
	send(socket, page, pageSize, 0);
}


static void
Send500Page(int socket)
{
	writestr(socket, "HTTP/1.1 500 Internal Server Error\n");
	writestr(socket, "Content-Type: text/html\n\n");	
	DWORD pageSize;
	char *page = (char *)LoadPageResource(IDR_500, &pageSize);
	send(socket, page, pageSize, 0);
}


static LPVOID
LoadPageResource(DWORD resourceId, LPDWORD resourceSize)
{
	HMODULE hModule = GetModuleHandle(NULL);
	HRSRC hResInfo = FindResource(hModule, MAKEINTRESOURCE(resourceId),
		MAKEINTRESOURCE(HTMLFILE));
	*resourceSize = SizeofResource(hModule, hResInfo);
	HGLOBAL rcData = LoadResource(hModule, hResInfo);
	return LockResource(rcData);
}


static void
ShowNotification(char *buffer, PNOTIFYICONDATA nid, DWORD dwNotificationType)
{
	nid->dwInfoFlags = dwNotificationType;
	Shell_NotifyIcon(NIM_MODIFY, nid);
}