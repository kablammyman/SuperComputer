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

	while (!done)
	{
		//conn.getData
			
	}
	conn.shutdown();


	return 0;
}


