#include <windows.h>
#include <string>
#include "NetworkConnection.h"

int NetworkConnection::connectToServer(string ip, int port, int socketType)
{
	ipAddy = ip;
	sockVersion = MAKEWORD(1, 1);
	WSAStartup(sockVersion, &wsaData);
	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);
	int yes = 1;
	portNumber = port;
	if (socketType == STREAM_SOCKET)
	{
		theSocket = socket(PF_INET, SOCK_STREAM, 0);
		setsockopt(theSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)); // lose the pesky "address already in use" error message
	}
	else
	{
		theSocket = socket(PF_INET, SOCK_DGRAM, 0);
		setsockopt(theSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)); // lose the pesky "address already in use" error message
	}

	if (theSocket == INVALID_SOCKET)
	{
		ReportError(WSAGetLastError(), "socket()");
		WSACleanup();
		return NETWORK_ERROR;
	}

	// Fill a SOCKADDR_IN struct with address information of host trying to conenct to
	remoteInfo.sin_family = AF_INET;
	remoteInfo.sin_addr.s_addr = inet_addr(ipAddy.c_str());
	remoteInfo.sin_port = htons(portNumber);// Change to network-byte order and insert into port field THIS HAS TO BE SET TO WHAT THE SERVER PORT IS LISTENING ON FOR DATAGRAM    
	memset(&(remoteInfo.sin_zero), '\0', 8); // zero the rest of the struct

	FD_SET(theSocket, &master);//for use with select()

							   // Connect to the server
	if (socketType == STREAM_SOCKET)//if we use this with datagram sokcets, we dont need to senttoand recvFrom...we use send and recv
	{
		int nret = connect(theSocket, (LPSOCKADDR)&remoteInfo, sizeof(struct sockaddr));
		//nret = connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

		if (nret == SOCKET_ERROR)
		{
			ReportError(WSAGetLastError(), "connect()");
			WSACleanup();
			return NETWORK_ERROR;
		}
	}

	// Successfully connected!
	return NETWORK_OK;
}
//------------------------------------------------------------------------------
void NetworkConnection::ReportError(int errorCode, std::string  whichFunc)
{
	string error = ("Call to " + whichFunc + " returned error ");
	std::cout << error << errorCode <<endl;
	//MessageBox(NULL, errorMsg, "socketIndication", MB_OK);
}
//------------------------------------------------------------------------------
int NetworkConnection::fillTheirInfo(SOCKADDR_IN *who, SOCKET daSocket)
{
	// Fill a SOCKADDR_IN struct with address information of comp trying to conenct to
	int length = sizeof(struct sockaddr);
	int otherCompInfo = getpeername(daSocket, (LPSOCKADDR)&who, &length);

	return otherCompInfo;
	/*
	htons() -- "Host to Network Short"
	htonl() -- "Host to Network Long"
	ntohs() -- "Network to Host Short"
	ntohl() -- "Network to Host Long"
	*/

	/*who->sin_family;
	who->sin_addr.s_addr;
	who->sin_port;*/
}
//------------------------------------------------------------------------------
int NetworkConnection::startServer(int numConnections, int port, int socketType)
{
	SOCKET newSocket;
	portNumber = port;
	sockVersion = MAKEWORD(1, 1);			// We'd like Winsock version 1.1
	WSAStartup(sockVersion, &wsaData);
	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);
	int yes = 1;
	int nret;
	
	numListeningConnections = numConnections;

	if (socketType == STREAM_SOCKET)
	{
		listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
		setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)); // lose the pesky "address already in use" error message
	}
	else
	{
		newSocket = socket(AF_INET, SOCK_DGRAM, 0);
		setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)); // lose the pesky "address already in use" error message
	}

	if (listeningSocket == INVALID_SOCKET) {
		ReportError(WSAGetLastError(), "socket()");		// Report the error with our custom function
		WSACleanup();				// Shutdown Winsock
		return NETWORK_ERROR;			// Return an error value
	}

	myInfo.sin_family = AF_INET;
	myInfo.sin_addr.s_addr = INADDR_ANY;	// Since this socket is listening for connections, any local address will do						                   
	myInfo.sin_port = htons(portNumber);		// Convert integer to network-byte order and insert into the port field		
	memset(&(myInfo.sin_zero), '\0', 8); // zero the rest of the struct 

	if (socketType == STREAM_SOCKET)// Bind the socket to our local server address
		nret = bind(listeningSocket, (LPSOCKADDR)&myInfo, sizeof(struct sockaddr));
	else
		nret = bind(newSocket, (LPSOCKADDR)&myInfo, sizeof(struct sockaddr));

	if (nret == SOCKET_ERROR) 
	{
		ReportError(WSAGetLastError(), "bind()");
		WSACleanup();
		return NETWORK_ERROR;
	}

	if (socketType == STREAM_SOCKET)// add the listener to the master set
		FD_SET(listeningSocket, &master);
	else
		FD_SET(newSocket, &master);

	if (socketType == STREAM_SOCKET)// Make the socket listen if we are a stream socket
	{
		nret = listen(listeningSocket, numListeningConnections);// Up to 10 connections may wait at any one time to be accept()'ed

		if (nret == SOCKET_ERROR) 
		{
			ReportError(WSAGetLastError(), "listen()");
			WSACleanup();
			return NETWORK_ERROR;
		}

	}
	else
		clientConnection.push_back(newSocket);


	waitingForClients = true;
	return NETWORK_OK;
}
//------------------------------------------------------------------------------
int NetworkConnection::waitForClientConnect()
{
	SOCKET curClient;
	int yes = 1;

	
	curClient = accept(listeningSocket, NULL, NULL);

	setsockopt(curClient, SOL_SOCKET, SO_REUSEADDR, (char*)&yes,sizeof(int)); // lose the pesky "address already in use" error message

	if (curClient == INVALID_SOCKET) 
	{
		ReportError(WSAGetLastError(), "accept()");
		WSACleanup();
		return NETWORK_ERROR;
	}

	fillTheirInfo(&remoteInfo, curClient);

	clientConnection.push_back(curClient);
	/*
	clientConnection = INVALID_SOCKET;
	while (clientConnection == INVALID_SOCKET)
	{
	clientConnection = accept(listeningSocket, NULL, NULL);
	//clientConnection = accept(listeningSocket,  (struct sockaddr *)&theirInfo ,&sin_size);
	}
	clientConnection = listeningSocket; 

	int sin_size = sizeof(struct sockaddr_in);
	//Address of a sockaddr structure...Address of a variable containing size of sockaddr struct
	clientConnection = accept(listeningSocket,  (struct sockaddr *)&theirInfo ,&sin_size);
	*/
	//sprintf(message,"we started a server listening on port %d",portNumber);
	// MessageBox(NULL, message, "Server message", MB_OK);
	return NETWORK_OK;
}
//------------------------------------------------------------------------------ 
int NetworkConnection::ServerBroadcast(char *string)//for stream sockets
{
	int howManySent = 0;
	for (size_t n = 0; n < clientConnection.size(); n++)
	{
		int nret = send(clientConnection[n], string, (int)strlen(string), 0);
		if (nret != -1)
			howManySent++;
	}

	return howManySent;
}
//------------------------------------------------------------------------------
void NetworkConnection::shutdown()
{
	closesocket(theSocket);

	for (size_t x = 0; x < clientConnection.size(); x++)
		closesocket(clientConnection[x]);
	closesocket(listeningSocket);

	// Shutdown Winsock
	WSACleanup();
}
//------------------------------------------------------------------------------ 
int NetworkConnection::sendData(SOCKET daSocket, char *msg)//for stream sockets
{
	int nret = send(daSocket, msg, (int)strlen(msg), 0);

	if (nret == SOCKET_ERROR) 
	{
		ReportError(WSAGetLastError(), "send()");
		return NETWORK_ERROR;
	}

	return nret;// nret contains the number of bytes sent

}

//------------------------------------------------------------------------------
int NetworkConnection::getData(SOCKET daSocket, char *msg)//for stream sockets
{
	int MAX_STRING_LENGTH = 256;
	int nret = recv(daSocket, msg, MAX_STRING_LENGTH, 0);// 256 = Complete size of buffer	  

	if (nret == SOCKET_ERROR) 
	{
		ReportError(WSAGetLastError(), "recv()");
		return NETWORK_ERROR;
	}

	return nret;// nret contains the number of bytes received
}

//------------------------------------------------------------------------------   
int NetworkConnection::sendData(SOCKET daSocket, char *msg, SOCKADDR_IN whomToSend)//for datagram sockets
{
	int structLength = sizeof(struct sockaddr);
	int nret = sendto(daSocket, msg, (int)strlen(msg), 0, (LPSOCKADDR)&whomToSend, structLength);

	if (nret == SOCKET_ERROR) 
	{
		ReportError(WSAGetLastError(), "send()");
		return NETWORK_ERROR;
	}

	return nret;// nret contains the number of bytes sent

}
//------------------------------------------------------------------------------
int NetworkConnection::getData(SOCKET daSocket, char *msg, SOCKADDR_IN whosSendingMeStuff)//for datagram sockets
{
	int structLength = sizeof(struct sockaddr);
	int MAX_STRING_LENGTH = 256;

	//nret = recvfrom(daSocket,string,MAX_STRING_LENGTH,0,(struct sockaddr *)&whosSendingMeStuff,&structLength);
	int nret = recvfrom(daSocket, msg, MAX_STRING_LENGTH, 0, (LPSOCKADDR)&whosSendingMeStuff, &structLength);

	if (nret == SOCKET_ERROR) 
	{
		ReportError(WSAGetLastError(), "recv()");
		return NETWORK_ERROR;
	}

	return nret;// nret contains the number of bytes received
}
//------------------------------------------------------------------------------   
int NetworkConnection::changeToNonBlocking(SOCKET daSocket)// Change the socket mode on the listening socket from blocking to non-block 
{
	ULONG NonBlock = 1;
	if (ioctlsocket(daSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
		return -1;
	return 0;
}
//------------------------------------------------------------------------------
/*std::string getServerInfo()
{
	int length =sizeof(struct sockaddr);
	int otherCompInfo = getpeername(theSocket,(LPSOCKADDR)&hostInfo,&length);
	return 
} */


///////////////////////
//part of start server code that worked, but unnesc
