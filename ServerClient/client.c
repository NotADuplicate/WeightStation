#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <fcntl.h>
#include <pthread.h>
#include <sys/select.h>
#include "client.h"

#define PORT 8080
#define WEIGHED 0
#define MAXLINE 10

int sockfd;
int *connection_ptr;
update_weighed_func client_update_ptr = NULL;
disconnect_func disconnect_func_ptr = NULL;

void* receive_thread_func(void* arg) {
    int sockfd = *((int*)arg);
    int buffer[MAXLINE];
    ssize_t valread;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        valread = recv(sockfd, buffer, sizeof(buffer), 0);
        if (valread <= 0) {
            if (valread == 0) {
                printf("Server disconnected\n");
                (*disconnect_func_ptr)();
                
            } else {
                perror("recv failed");
            }
            close(sockfd);
            return NULL;
        }

        int num_ints = valread / sizeof(int);
        printf("Received %d integers: ", num_ints);
        for (int i = 0; i < num_ints; i++) {
            printf("%d ", buffer[i]);
        }
        printf("\n");
        
        if(buffer[0] == 0) { //got weight update
            (*client_update_ptr)(buffer[2], buffer[1]);
        }
    }

    return NULL;
}

int check_connection() {
    printf("Checking connection\n");
    char buffer[1] = {1};  // dummy buffer
    if (send(sockfd, buffer, sizeof(buffer), MSG_NOSIGNAL) == -1) {
        printf("Sent correctly\n");
        return 0;
    } else {
        printf("Sent incorrectly\n");
        return 1;
    }
}

void send_weighed(int player, int team) {
    int message[3] = {WEIGHED, team, player};

    int sent = send(sockfd, message, sizeof(message), 0); 
    printf("Message sent: %d %d\n", player, team);
    printf("Message sent correctly: %i\n",sent);
}

int create_client(char *ip_address, int *window_connected) { //return 0 if failed
    printf("Creating client\n\n\n\n");
    struct sockaddr_in servaddr; 
    connection_ptr = window_connected;

    // Create socket file descriptor 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        perror("socket creation failed"); 
        return 0;
    } 

    // Set socket to non-blocking
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    memset(&servaddr, 0, sizeof(servaddr)); 

    // Fill in server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = inet_addr(ip_address); // The IP address of the other device

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    // Set up the file descriptor set.
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);

    // Set up the struct timeval for the timeout.
    struct timeval tv;
    tv.tv_sec = 5;  // 5 second timeout
    tv.tv_usec = 0;

    // wait until timeout or data received.
    int rv = select(sockfd + 1, NULL, &fdset, NULL, &tv);
    if (rv == -1) {
        perror("select"); // error occurred in select()
        return 0;
    } else if (rv == 0) {
        printf("connection timed out.\n");
        return 0;
    } else {
        // one or both of the descriptors have data
        int error;
        socklen_t len = sizeof (error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

        if (error == 0) {
            // socket is successfully connected
            fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) & ~O_NONBLOCK);
            *connection_ptr = 1;
            
            //start thread
            pthread_t receive_thread;
            pthread_create(&receive_thread, NULL, receive_thread_func, &sockfd);
            pthread_detach(receive_thread);
            
            return 1;
        } else {
            // error occurred, connection failed
            close(sockfd);
            perror("connection failed");
            return 0;
        }
    }
    return -1;
}

struct client_args {
    char* ip_address;
    int* window_connected;
};

void* create_client_thread_func(void* arg) {
    struct client_args* args = (struct client_args*)arg;
    printf("Client thread first\n");
    while (create_client(args->ip_address, args->window_connected) == 0) {
        printf("Trying client thread again\n");
        sleep(5);  // wait 5 seconds between retries
    }
    free(arg);
    return NULL;
}

void start_client(char* ip_address, int* window_connected) {
    struct client_args* args = malloc(sizeof(struct client_args));
    args->ip_address = ip_address;
    args->window_connected = window_connected;

    pthread_t client_thread;
    pthread_create(&client_thread, NULL, create_client_thread_func, args);
    pthread_detach(client_thread);
}

void close_client() {
	close(sockfd);
}
