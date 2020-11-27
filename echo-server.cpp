#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <set>
#include <thread>
#include <mutex>
#include <stdlib.h>
#include <signal.h>
using namespace std;
#define BUFFERSIZE 1024
struct SetInt:set<int>{
	mutex mut;
	void lock(){mut.lock();}
	void unlock(){mut.unlock();}
};
SetInt client_fd;
bool broadcast = false;
bool echo = false;
void recvThread(int cli_sd){
	printf("Connected and recvThread is working!\n");
	char message[BUFFERSIZE];
	while(true){
		ssize_t res = recv(cli_sd, message, BUFFERSIZE - 1, 0);
		if(res==0 || res==-1){
			perror("recv failed");
            break;
		}
		message[res] = '\0';
		printf("message : %s\n",message);
		if(echo == true && broadcast == false){
			res = send(cli_sd, message, res, 0);
			if(res == 0 || res == -1){
				perror("send failed");
	        	break;
			}
		}
		if(echo == true && broadcast == true){
			client_fd.lock();
			printf("broadcast send to %d number of clients\n",(int)client_fd.size());
			set<int>::iterator it;
			for(it=client_fd.begin();it!=client_fd.end();){
				res = send((*it), message, res, 0);
				if(res == 0 || res == -1){
					perror("send failed");
					it = client_fd.erase(it);
	            	continue;
				}
				it++;
			}
			client_fd.unlock();
			if(client_fd.find(cli_sd)==client_fd.end()){
				break;
			}
		}
	}
	if(broadcast==true){
		client_fd.lock();
		client_fd.erase(cli_sd);
		client_fd.unlock();
	}
	close(cli_sd);
}
int main(int argc, char **argv) {
	if (argc < 2 || argc > 4) {
		printf("echo-server:\n");
		printf("syntax : echo-server <port> [-e[-b]]\n");
		printf("sample : echo-server 1234 -e -b \n");	
		return -1;
	}
	if(argc >= 3){
		echo = true;
	}
	if(argc == 4){
		broadcast = true;
	}
	int server_socket;
	int opt = 1;
	struct sockaddr_in server_address;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		perror("socket");
		return -1;
	}
	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))==-1){
		perror("setsockopt");
		return -1;
	}
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(atoi(argv[1]));
	if(bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address))==-1){
		perror("bind");
		return -1;
	}
	if(listen(server_socket, 5)==-1){
		perror("listen");
		return -1;
	}

	while(true){
		struct sockaddr_in client_address;
		socklen_t client_address_size = sizeof(client_address);
		int cli_sd = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);
		if (cli_sd == -1) {
			perror("accept");
			break;
		}
		if(broadcast==true){
			client_fd.lock();
			client_fd.insert(cli_sd);
			client_fd.unlock();
		}
		thread* cl_th = new thread(recvThread, cli_sd);
		cl_th->detach();
	}
	close(server_socket);
	return 0;
}