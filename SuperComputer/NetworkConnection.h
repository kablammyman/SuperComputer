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
struct RemoteComputerConnection
{
	SOCKET theSocket;
	int portNumber = 0;
	string ipAddy;
	SOCKADDR_IN remoteInfo;
};

class NetworkConnection
{
	enum SOCKET_TYPE
	{
		DATAGRAM_SOCKET = 0,
		STREAM_SOCKET 
	};

	LPHOSTENT hostEntry;
	SOCKADDR_IN myInfo;
	
	WORD sockVersion;
	WSADATA wsaData;
	
	int sockfd;
	struct timeval tv;

	fd_set master;   // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	
	bool waitingForClients = false;
	SOCKET listeningSocket;
	int numListeningConnections;
	vector<RemoteComputerConnection> remoteConnections;

public:
	void ReportError(int errorCode, std::string  whichFunc);
	int fillTheirInfo(SOCKADDR_IN *who, SOCKET daSocket);
	
	int waitForFirstClientConnect();
	int waitForClientAsync();

	int startServer(int numConnections, int port, int socketType = STREAM_SOCKET);
	int connectToServer(string ip, int port, int socketType = STREAM_SOCKET);
	void shutdown();
	
	
	//for stream sockets
	int ServerBroadcast(const char *msg);

	int sendData(int socketIndex, const char *msg);
	int getData(int socketIndex, char *msg, int dataSize);
	

	//for datagram sockets
	int sendData(SOCKET daSocket, const char *msg, SOCKADDR_IN whomToSend);
	int getData(SOCKET daSocket, char *msg, SOCKADDR_IN whosSendingMeStuff);

	int changeToNonBlocking(SOCKET daSocket);
	size_t getNumConnections() {return remoteConnections.size();}
	SOCKET getSocket(int index) { return remoteConnections[index].theSocket; }
	bool hasRecivedData(int index);
	void closeConnection(int index);
};


#endif //INC_NETWORKCONNECTION_H