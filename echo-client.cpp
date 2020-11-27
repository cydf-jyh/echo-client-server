#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include <stdlib.h>
#include <signal.h>
using namespace std;
#define BUFFERSIZE 1024
int close_fd;
void recvThread(int client_fd) {
	printf("RecvThread is working!\n");
	char buf[BUFFERSIZE];
	while (true) {
		ssize_t res = recv(client_fd, buf, BUFFERSIZE - 1, 0);
		if (res == 0 || res == -1) {
			perror("recv failed!");
			break;
		}
		buf[res] = '\0';
		printf("message sent from server : %s\n",buf);
	}
	printf("disconnected!\n");
	close(client_fd);
	exit(0);
}
int main(int argc, char **argv) {
	int client_fd;
	struct sockaddr_in server_address;
	if (argc != 3) {
		printf("echo-client:\n");
		printf("syntax : echo-client <ip> <port>\n");
		printf("sample : echo-client 127.0.0.1 1234\n");
		return 0;
	}
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_fd == -1){
		printf("socket failed!\n");
		return 0;
	}
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	if(!inet_aton(argv[1], &server_address.sin_addr)){
		printf("Invalid Server Ip address!\n");
		return 0;
	}
	server_address.sin_port = htons(atoi(argv[2]));
	int flag=connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address));
	if(flag==-1){
		printf("Connection disabled!\n");
		return 0;
	}
	printf("Client program connection established\n\n");
	close_fd=client_fd;
	thread thread_client_recv(recvThread, client_fd);
	thread_client_recv.detach();
	while(true){
		char message[BUFFERSIZE];
		int message_length;
		scanf("%s",message);
		message_length=strlen(message);
		ssize_t res = send(client_fd, message, message_length, 0);
		if (res == 0 || res == -1) {
			perror("send failed!");
			break;
		}
	}
	close(client_fd);
	return 0;
}