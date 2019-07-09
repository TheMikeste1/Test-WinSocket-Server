/*****************************************************************************
* Author:
*    Michael Hegerhorst
* Summary:
*
*
************************************************************************/
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
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

#define DEFAULT_PORT "6789"
#define BACKLOG 10
#define DEFAULT_BUFLEN 512

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET)
   {
      return &(((struct sockaddr_in*)sa)->sin_addr);
   }

   return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

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

    int resultCode;
	if ((resultCode = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		cout << "WSAStartup failed: " << resultCode << endl;
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

	//2.1. The getaddrinfo function is used to determine the values
	//   in the sockaddr structure:
	if ((resultCode = getaddrinfo(NULL, port, &hints, &servinfo))
		!= 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(resultCode));
		WSACleanup();
		return 1;
	}



	//2.2. Create a SOCKET object called listenSocket for the server
	//   to listen for client connections.
	//2.3. Call the socket function and return its value to
	//   the listenSocket variable.
	SOCKET listenSocket = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
								 servinfo->ai_protocol);
	//SOCKET = unsigned integer pointer

	//2.4. Check for errors.
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(servinfo);
		WSACleanup();
		return 1;
	}

	// 3. Bind the socket.
	//https://docs.microsoft.com/en-us/windows/desktop/winsock/binding-a-socket
	if ((resultCode = bind(listenSocket, servinfo->ai_addr, servinfo->ai_addrlen))
		== SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(servinfo);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
	}

	freeaddrinfo(servinfo); //"all done with this structure" -rpsServer.cpp

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

	//5.1. Create a temporary SOCKET
	SOCKET client1;

	//5.2. Connect
	cout << "server: waiting for connections...\n";
	
	//This is to gather information about the client's IP.
	struct sockaddr_storage their_address;
	socklen_t size = sizeof their_address;
	char s[INET6_ADDRSTRLEN];

	if ((client1 = accept(listenSocket, (struct sockaddr *) &their_address, &size))
		== INVALID_SOCKET)
	{
		printf("accept failed: %d\n", WSAGetLastError());
	    closesocket(listenSocket);
	    WSACleanup();
	    return 1;
	}

	//This sets s to the IP of the client
	inet_ntop(their_address.ss_family,
      get_in_addr((struct sockaddr *)&their_address),
      s, sizeof s);

	cout << "Got connection from: " << s << endl;

	closesocket(listenSocket); //We no longer need the listenSocket after
							   //all wanted clients have connected.


	// 6. Receive data.
	//https://docs.microsoft.com/en-us/windows/desktop/winsock/receiving-and-sending-data-on-the-server
	do
	{
		int recvBufferLen = DEFAULT_BUFLEN;
		char* received = new char[recvBufferLen + 1];

		resultCode = recv(client1, received, recvBufferLen, 0);
		received[resultCode] = '\0';

		if (resultCode > 0)
		{
			cout << received;
		}
		else if (resultCode == 0)
		{
			cout << "Connection closed by client\n";
		}
		else
		{
			printf("recv failed: %d\n", WSAGetLastError());
	        closesocket(client1);
	        WSACleanup();
	        return 1;
		}
	} while (resultCode != 0);

	// 7. Disconnect.
	//https://docs.microsoft.com/en-us/windows/desktop/winsock/disconnecting-the-server
	//7.1
	if ((resultCode = shutdown(client1, SD_SEND)) == SOCKET_ERROR)
	{
		printf("shutdown failed: %d\n", WSAGetLastError());
	    closesocket(client1);
	    WSACleanup();
	    return 1;
	}

	//7.2
	//Cleanup
	closesocket(client1);
	WSACleanup();

	return 0;
}
