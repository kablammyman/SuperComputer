#include <string>
#include <iostream>

#include <windows.h>
#include <winsock.h>
#include <stdio.h>

#include <thread>

#include <vector>
#include <string>

#include "NetworkConnection.h"

SERVICE_STATUS        g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler (DWORD);
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);

#define SERVICE_NAME  L"My Sample Service"

using namespace std;

#define DEFAULT_BUFLEN 512
#define NETWORK_ERROR -1
#define NETWORK_OK     0
//SOMAXCONN = socket max amunt of connections?

vector<string> stringVec;
	bool done = false;
	bool isServer;
	string ip = "127.0.0.1";
	int port = 2345;
	NetworkConnection conn;




int main (int argc, TCHAR *argv[])
{
    OutputDebugString(L"My Sample Service: Main: Entry");
		
	if (argc > 1)
		isServer = true;
	else
		isServer = false;

    SERVICE_TABLE_ENTRY ServiceTable[] = 
    {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
    {
       OutputDebugString(L"My Sample Service: Main: StartServiceCtrlDispatcher returned error");
       return GetLastError ();
    }

    OutputDebugString(L"My Sample Service: Main: Exit");
    return 0;
}

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;

    OutputDebugString(L"My Sample Service: ServiceMain: Entry");

    g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL) 
    {
        OutputDebugString(L"My Sample Service: ServiceMain: RegisterServiceCtrlHandler returned error");
        goto EXIT;
    }

    // Tell the service controller we are starting
    ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) 
    {
        OutputDebugString(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
    }

    /* 
     * Perform tasks neccesary to start the service here
     */
    OutputDebugString(L"My Sample Service: ServiceMain: Performing Service Start Operations");

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) 
    {
        OutputDebugString(L"My Sample Service: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error");

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
	    {
		    OutputDebugString(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
	    }
        goto EXIT; 
    }    

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    OutputDebugString(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
    }

    // Start the thread that will perform the main task of the service
    HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    OutputDebugString(L"My Sample Service: ServiceMain: Waiting for Worker Thread to complete");

    // Wait until our worker thread exits effectively signaling that the service needs to stop
    WaitForSingleObject (hThread, INFINITE);
    
    OutputDebugString(L"My Sample Service: ServiceMain: Worker Thread Stop Event signaled");
    
    
    /* 
     * Perform any cleanup tasks
     */
    OutputDebugString(L"My Sample Service: ServiceMain: Performing Cleanup Operations");

    CloseHandle (g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    OutputDebugString(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
    }
    
    EXIT:
    OutputDebugString(L"My Sample Service: ServiceMain: Exit");

    return;
}

VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
{
    OutputDebugString(L"My Sample Service: ServiceCtrlHandler: Entry");

    switch (CtrlCode) 
	{
     case SERVICE_CONTROL_STOP :

        OutputDebugString(L"My Sample Service: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request");

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
           break;

        /* 
         * Perform tasks neccesary to stop the service here 
         */
        
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(L"My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error");
		}

        // This will signal the worker thread to start shutting down
        SetEvent (g_ServiceStopEvent);

        break;

     default:
         break;
    }

    OutputDebugString(L"My Sample Service: ServiceCtrlHandler: Exit");
}


DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
    int counter = 5;
	int iResult;
	string msg = "hello slave...I mean, client!";
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	srand(time(NULL));
	string name = "cpu" + to_string(rand() % 100);
	if (isServer)
		name = "server";
	
	OutputDebugString(L"My Sample Service: ServiceWorkerThread: Entry");

	if (isServer)
		{
			cout << "starting server\n";
			conn.startServer(SOMAXCONN,port);
			conn.waitForFirstClientConnect();
		}
		else
		{
			cout << "connecting to " << ip << "\n";
			if (conn.connectToServer(ip,port) == NETWORK_ERROR)
			{
				exit(-1);
			}
		}
	
		cout << "connected!\n";

    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {        
			if (isServer)
			{
				conn.waitForClientAsync();
				if (counter >= 5)
				{
					cout << "sending msg \n";
					conn.ServerBroadcast(msg.c_str());
					counter = 0;
				}
				counter++;
			}
			int x = conn.getNumConnections();
			for (int i = 0; i < x;  i++)
			{
				if (conn.hasRecivedData(i))
				{
					iResult = conn.getData(i, recvbuf, DEFAULT_BUFLEN);
					if (iResult > 0)
					{
						recvbuf[iResult] = '\0';
						printf("%s -> %d bytes.\n", recvbuf, iResult);
						if(!isServer)
							conn.sendData(i, name.c_str());
					}
					else if (iResult == 0)
						done = true;
					else
						printf("recv failed: %d\n", WSAGetLastError());
				}
			}
		
		
	}

    OutputDebugString(L"My Sample Service: ServiceWorkerThread: Exit");
	conn.shutdown();
    return ERROR_SUCCESS;
}
int oldmain(int argc, char * argv[])
{
	
	


	return 0;
}


