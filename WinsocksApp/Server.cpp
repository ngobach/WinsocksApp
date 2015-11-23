#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEBUG false

WSADATA wsaData;
SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket;

/**
* Return 1 on failure
**/
int init(){
	int iResult;
	addrinfo *result = NULL, hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof (hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);
	return 0;
}

int accept(){
	puts("Listening...");
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	puts("Accepting...");
	ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, NULL, NULL);
	puts("Accepted!");
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}


int loop(){
#define DEFAULT_BUFLEN 512

	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			if (DEBUG)
				printf("Bytes received: %d\n", iResult);
			puts(recvbuf);
			puts("Your message: (blank for quit)");
			gets(recvbuf);
			if (strlen(recvbuf) == 0){
				return 0;
			}
			iSendResult = send(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			if (DEBUG)
				printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);
}

int shutdown(){
	int result = shutdown(ListenSocket, SD_SEND);
	if (result == SOCKET_ERROR){
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}

int main_() {
	puts("Initializing ...");
	if (init()){
		return 1;
	}
	puts("Waiting for client ...");
	if (accept()){
		return 2;
	}
	puts("Client connected. Waiting for message");
	if (loop()){
		return 3;
	}
	puts("Shuting down ...");
	shutdown();
	return 0;
}
int main(){
	main_();
	getchar();
	return 0;
}