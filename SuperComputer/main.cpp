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
	bool isServer = true;
	string ip;
	int port;
	NetworkConnection conn;

	if (isServer)
	{
		conn.startServer(SOMAXCONN);
		conn.waitForClientConnect();
	}
	else
		conn.connectToServer(ip);
	
	while (!done)
	{

			
	}
	conn.shutdown();


	return 0;
}


