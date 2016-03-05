#include <string>
#include <iostream>

#include <windows.h>
#include <winsock.h>
#include <stdio.h>

#include <thread>
#include <mutex>


#include <vector>
#include <string>

#include "NetworkConnection.h"

using namespace std;

#define DEFAULT_BUFLEN 512
#define NETWORK_ERROR -1
#define NETWORK_OK     0
//SOMAXCONN = socket max amunt of connections?

int main(int argc, char * argv[])
{
	vector<string> stringVec;
	bool done = false;
	bool isServer;
	string ip = "127.0.0.1";
	int port = 2345;
	NetworkConnection conn;

	if (argc > 1)
		isServer = true;
	else
		isServer = false;

	if (isServer)
	{
		cout << "starting server\n";
		conn.startServer(SOMAXCONN,port);
		conn.waitForClientConnect();
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
	int counter = 500;
	int iResult;
	string msg = "hello slave...I mean, client!";
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	string name = "cpu" + to_string(rand() % 100);
	if (isServer)
		name = "server";
	while (!done)
	{
		if (isServer)
		{
			if (counter >= 500)
			{
				conn.ServerBroadcast(msg.c_str());
				counter = 0;
			}
			counter++;
		}

		iResult = conn.getData(0,recvbuf, DEFAULT_BUFLEN);
		if (iResult > 0)
		{
			recvbuf[iResult] = '\0';
			printf("%s -> %d bytes.\n", recvbuf, iResult);
			conn.sendData(, name.c_str());
		}
		else if (iResult == 0)
			done = true;
		else
			printf("recv failed: %d\n", WSAGetLastError());
	}
	conn.shutdown();


	return 0;
}


