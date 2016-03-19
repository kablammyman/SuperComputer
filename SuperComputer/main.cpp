//tread for adding and removing connections?
//and a thread for work?

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


string ip = "127.0.0.1";
int port = 2345;
NetworkConnection conn;


//sc create "My Sample Service" binPath= C:\SampleService.exe

int main (int argc, TCHAR *argv[])
{
    OutputDebugString(L"My Sample Service: Main: Entry");
		
    SERVICE_TABLE_ENTRY ServiceTable[] = 
    {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
    {
       wstring errorText = L"My Sample Service: Main: StartServiceCtrlDispatcher returned error: " + to_wstring(GetLastError());
		OutputDebugString(errorText.c_str());
       return -1;
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

void connectToRemote()
{
	cout << "connecting to " << ip << "\n";
	if (conn.connectToServer(ip, port) == NETWORK_ERROR)
	{
		exit(-1);
	}
}

void broadcastMsg(string msg)
{
	cout << "sending msg \n";
	conn.ServerBroadcast(msg.c_str());
}

int parseCommand(string cmd)
{
	if(cmd == "time")
		return 0;
	else if(cmd == "name")
		return 1;

	return -1;
}

DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
    int counter = 5;
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int numConn = 0;
	
	OutputDebugString(L"My Sample Service: ServiceWorkerThread: Entry");

	cout << "starting server\n";
	conn.startServer(SOMAXCONN,port);
	//conn.waitForFirstClientConnect();
	//cout << "connected!\n";

    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {        
			conn.waitForClientAsync();
				
			numConn = (int)conn.getNumConnections();
			for (int i = 0; i < numConn;  i++)
			{
				if (conn.hasRecivedData(i))
				{
					iResult = conn.getData(i, recvbuf, DEFAULT_BUFLEN);
					if (iResult > 0)
					{
						recvbuf[iResult] = '\0';
						printf("%s -> %d bytes.\n", recvbuf, iResult);
						
						switch (parseCommand(recvbuf))
						{
						case 0:
							time_t rawtime;
							struct tm * timeinfo;

							time(&rawtime);
							timeinfo = localtime(&rawtime);
							//printf("The current date/time is: %s", asctime(timeinfo));
							conn.sendData(i, asctime(timeinfo));
							break;
						case 1:
							conn.sendData(i, "Fred");
							break;
						default:
							conn.sendData(i, "unknown command");
							break;
						}
							
					}
					//client disconnected
					else if (iResult == 0)
						conn.closeConnection(i);

					else
						printf("recv failed: %d\n", WSAGetLastError());
				}
			}
		
		
	}

    OutputDebugString(L"My Sample Service: ServiceWorkerThread: Exit");
	conn.shutdown();
    return ERROR_SUCCESS;
}

