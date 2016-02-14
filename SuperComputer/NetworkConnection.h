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


class NetworkConnection
{
protected:
	enum SOCKET_TYPE
	{
		DATAGRAM_SOCKET = 0,
		STREAM_SOCKET 
	};

	WORD sockVersion;
	WSADATA wsaData;

	int sockfd;
	int portNumber = 0;

	fd_set master;   // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	

public:
	void ReportError(int errorCode, std::string  whichFunc);
	int fillTheirInfo(SOCKADDR_IN *who, SOCKET daSocket);
	virtual int init(int socketType) = 0;
	virtual void shutdown() = 0;
	
	
	//for stream sockets
	int getData(SOCKET daSocket, char *string);
	int sendData(SOCKET daSocket, char *string);

	//for datagram sockets
	int sendData(SOCKET daSocket, char *string, SOCKADDR_IN whomToSend);
	int getData(SOCKET daSocket, char *string, SOCKADDR_IN whosSendingMeStuff);

	int changeToNonBlocking(SOCKET daSocket);

	
};

class NetworkServer : public NetworkConnection
{
private:
	bool waitingForClients = false;
	SOCKADDR_IN myInfo;
	SOCKADDR_IN theirInfo;
	vector<SOCKET> clientConnection;
	SOCKET listeningSocket;
	int numListeningConnections;
	int waitForClientConnect();
public:
	NetworkServer(int numConnections);
	int ServerBroadcast(char *string);
	virtual int init(int socketType = STREAM_SOCKET);
	virtual void shutdown();
};

class NetworkClient : public NetworkConnection
{
private:
	LPHOSTENT hostEntry;
	SOCKET theSocket;
	SOCKADDR_IN hostInfo;
	string ipAddy;
public:
	NetworkClient(string ip);
	virtual int init(int socketType = STREAM_SOCKET);
	virtual void shutdown();
};
#endif //INC_NETWORKCONNECTION_H