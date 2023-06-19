#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "../Curl/roster.h"
#include <signal.h>
#include "server.h"

#define PORT 8080
#define MAXLINE 1024 
#define MAX_CLIENTS 100
#define WEIGHED 0

int client_sockets[MAX_CLIENTS];
int connected_clients = 0;
int server_fd;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t server_thread;
update_weighed_func update_weighed_ptr = NULL;


void broadcast(int* message) {
    for (int i = 0; i < connected_clients; i++) {
        send(client_sockets[i], message, sizeof(int)*3, 0);
    }
}

void handle_sigint(int sig) 
{ 
    printf("Caught signal %d\n", sig);
    if (server_fd != -1) {
        close(server_fd);
    }
    exit(0);
} 

void* handle_client(void* arg) {
    int new_socket = *((int*)arg);
    int buffer[3];
    int valread;
    
    printf("Handling client\n");

    while ((valread = read(new_socket, buffer, sizeof(int) * 3)) > 0) {
        printf("Received: %i %i\n", buffer[0], buffer[1]);
        if(buffer[0] == WEIGHED) {
	    broadcast(buffer);
	    if (update_weighed_ptr != NULL) {
		(*update_weighed_ptr)(buffer[2], buffer[1]);
	    }
	}
        memset(buffer, 0, sizeof(buffer));
	
    }

    printf("Client disconnected\n");
    close(new_socket);

    free(arg);  // free the allocated memory

    return NULL;
}

void *server_thread_func() {
    int new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    signal(SIGINT, handle_sigint);

	while (1) {
		printf("\nWaiting for a connection...\n");

		int* new_sock_ptr = malloc(sizeof(int));  // allocate an int on the heap
		if ((*new_sock_ptr = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
			perror("accept");
			free(new_sock_ptr);  // don't forget to free the allocated memory
			exit(EXIT_FAILURE);
		}

		printf("Connection accepted\n");

		pthread_mutex_lock(&lock);
		client_sockets[connected_clients++] = *new_sock_ptr;  // use *new_sock_ptr here
		pthread_mutex_unlock(&lock);

		pthread_t thread_id;
		pthread_create(&thread_id, NULL, handle_client, new_sock_ptr);  // pass new_sock_ptr directly
		pthread_detach(thread_id);
	}

    return 0;
}

int initialize_server() {
    if(pthread_create(&server_thread, NULL, server_thread_func, NULL) != 0) {
        perror("Failed to start server thread");
        return EXIT_FAILURE;
    }
    return 0;
}

