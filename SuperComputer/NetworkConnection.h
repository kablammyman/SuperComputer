#ifndef INC_NETWORKCONNECTION_H
#define INC_NETWORKCONNECTION_H

#include <windows.h>
#include <winsock.h>
#include <iostream>
#include <vector>
#include <string>

#define NETWORK_ERROR -1
#define NETWORK_OK     0

using namespace std;
/*
change it so that each "connection" (aka server or client) has its own
params including port num

have an array of that class*/

class NetworkConnection
{
	enum SOCKET_TYPE
	{
		DATAGRAM_SOCKET = 0,
		STREAM_SOCKET 
	};

	LPHOSTENT hostEntry;
	SOCKET theSocket;
	SOCKADDR_IN myInfo;
	SOCKADDR_IN remoteInfo;

	WORD sockVersion;
	WSADATA wsaData;
	string ipAddy;

	int sockfd;
	int portNumber = 0;

	fd_set master;   // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	
	bool waitingForClients = false;
	SOCKET listeningSocket;
	int numListeningConnections;
	vector<SOCKET> clientConnection;

public:
	void ReportError(int errorCode, std::string  whichFunc);
	int fillTheirInfo(SOCKADDR_IN *who, SOCKET daSocket);
	
	int waitForClientConnect();
	
	int startServer(int numConnections, int port, int socketType = STREAM_SOCKET);
	int connectToServer(string ip, int port, int socketType = STREAM_SOCKET);
	void shutdown();
	
	
	//for stream sockets
	int ServerBroadcast(char *string);
	int getData(SOCKET daSocket, char *msg);
	//int getData(char *msg);
	int sendData(SOCKET daSocket, char *msg);

	//for datagram sockets
	int sendData(SOCKET daSocket, char *msg, SOCKADDR_IN whomToSend);
	int getData(SOCKET daSocket, char *msg, SOCKADDR_IN whosSendingMeStuff);

	int changeToNonBlocking(SOCKET daSocket);

	
};


#endif //INC_NETWORKCONNECTION_H