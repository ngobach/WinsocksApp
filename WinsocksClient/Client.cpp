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
#define DEFAULT_HOST "127.0.0.1"
#define DEBUG true
WSADATA wsaData;
SOCKET MySocket = INVALID_SOCKET;

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
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(DEFAULT_HOST, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	MySocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (MySocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	iResult = connect(MySocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(MySocket);
		MySocket = INVALID_SOCKET;
	}
	freeaddrinfo(result);
	if (MySocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
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
	strcpy(recvbuf, "Hello Server!");
	iSendResult = send(MySocket, recvbuf, DEFAULT_BUFLEN - 1, 0);
	puts("Sending hello message!");
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(MySocket);
		WSACleanup();
		return 1;
	}
	do {
		iResult = recv(MySocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			if (DEBUG)
				printf("Bytes received: %d\n", iResult);
			puts(recvbuf);
			puts("Your message: (blank for quit)");
			gets(recvbuf);
			if (strlen(recvbuf) == 0){
				return 0;
			}
			iSendResult = send(MySocket, recvbuf, DEFAULT_BUFLEN, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(MySocket);
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
			closesocket(MySocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);
	return 0;
}

int shutdown(){
	int result = shutdown(MySocket, SD_SEND);
	if (result == SOCKET_ERROR){
		closesocket(MySocket);
		WSACleanup();
		return 1;
	}
	closesocket(MySocket);
	WSACleanup();
	return 0;
}

int main_() {
	puts("Initializing ...");
	if (init()){
		getchar();
		return 1;
	}
	puts("Connected to server. Waiting for response");
	if (loop()){
		return 2;
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