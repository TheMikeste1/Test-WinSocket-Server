/***********************************************************************
* Author:
*    Michael Hegerhorst
* Summary:
*
*
************************************************************************/

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

#define DEFAULT_PORT "6789"
#define BACKLOG 10


/*****************************************************************************
 * Help from
 * https://docs.microsoft.com/en-us/windows/desktop/winsock/about-clients-and-servers
 * 
 * SERVER STEPS
 *  1. Initialize Winsock.
 *  2. Create a socket.
 *  3. Bind the socket.
 *  4. Listen on the socket for a client.
 *  5. Accept a connection from a client.
 *  6. Receive and send data.
 *  7. Disconnect.
 *
 *****************************************************************************/

/*****************************************************************************
 * main()
 * This function pilots the program.
 *****************************************************************************/
int main()
{

	//SERVER STEP
	// 1. Initialize Winsock.
	//https://docs.microsoft.com/en-us/windows/desktop/winsock/creating-a-basic-winsock-application
	WSADATA wsaData;

    int* result = new int(0);
	if ((*result = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		cout << "WSAStartup failed: " << *result << endl;
		return 1;
	}


	// 2. Create a socket.
	//https://docs.microsoft.com/en-us/windows/desktop/winsock/creating-a-socket-for-the-server
	struct addrinfo *servinfo = NULL;
	struct addrinfo *p = NULL;
	struct addrinfo hints;

	PCSTR port = DEFAULT_PORT; //PCSTR = const char* (Pointer Constant STRing)

	ZeroMemory(&hints, sizeof(hints)); //This macro fills hints with zeroes.
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags    = AI_PASSIVE; 

	//1. The getaddrinfo function is used to determine the values 
	//   in the sockaddr structure:
	if ((*result = getaddrinfo(NULL, port, &hints, &servinfo))
		!= 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(*result));
		WSACleanup();
		return 1;
	}

	

	//2. Create a SOCKET object called listenSocket for the server 
	//   to listen for client connections.
	//3. Call the socket function and return its value to 
	//   the listenSocket variable.
	SOCKET listenSocket = socket(servinfo->ai_family, 
							 servinfo->ai_socktype, 
								 servinfo->ai_protocol);
	//SOCKET == unsigned integer pointer

	//4. Check for errors.
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(servinfo);    
		WSACleanup();
		return 1;
	}
	
	// 3. Bind the socket.
	//https://docs.microsoft.com/en-us/windows/desktop/winsock/binding-a-socket
	if ((*result = bind(listenSocket, servinfo->ai_addr, servinfo->ai_addrlen))
		== SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(servinfo);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
	}

	freeaddrinfo(servinfo); //"all done with this structure" -rpsServer.cpp
	
	delete result; //We don't need result anymore. Let's clear it.

	// 4. Listen on the socket.
	//https://docs.microsoft.com/en-us/windows/desktop/winsock/listening-on-a-socket
	if (listen(listenSocket, BACKLOG) == SOCKET_ERROR)
	{
		printf( "Listen failed with error: %ld\n", WSAGetLastError() );
		closesocket(listenSocket);
	    WSACleanup();
	    return 1;
	}

	// 5. Accept a connection.
	//https://docs.microsoft.com/en-us/windows/desktop/winsock/accepting-a-connection

	//1. Create a temporary SOCKET
	SOCKET client1;

	//2. Connect
	if ((client1 = accept(listenSocket, NULL, NULL)) 
		== INVALID_SOCKET)
	{
		printf("accept failed: %d\n", WSAGetLastError());
	    closesocket(listenSocket);
	    WSACleanup();
	    return 1;
	}

	

	cout << "server: waiting for connections...\n";
	
	return 0;
}
