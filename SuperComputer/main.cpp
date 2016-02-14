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

#define NETWORK_ERROR -1
#define NETWORK_OK     0


int main(int argc, char * argv[])
{
	std::vector<std::string> stringVec;

	NetworkServer server(SOMAXCONN);//SOMAXCONN = socket max amunt of connections?
	server.init();

	/*if (waitingForClients)
	waitForClientConnect();*/
	
	server.shutdown();


	return 0;
}


