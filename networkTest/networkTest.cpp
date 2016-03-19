// networkTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NetworkConnection.h"


#define DEFAULT_BUFLEN 512

int main()
{
	vector<string> stringVec;
	bool done = false;
	bool isServer = false;
	string ip = "127.0.0.1";
	int port = 2345;
	NetworkConnection conn;

	conn.connectToServer(ip, port);

	int i = 0;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	conn.sendData(i, "time");
	do {
		if (conn.hasRecivedData(i))
		{
			int iResult = conn.getData(i, recvbuf, DEFAULT_BUFLEN);
			if (iResult > 0)
			{
				recvbuf[iResult] = '\0';
				printf("%s -> %d bytes.\n", recvbuf, iResult);

			}
			//client disconnected
			else if (iResult == 0)
			{
				printf("server disconnected\n");
			}
			else
				printf("recv failed: %d\n", WSAGetLastError());

			done = true;
		}
	} while (!done);

	conn.shutdown();
	return 0;
}

