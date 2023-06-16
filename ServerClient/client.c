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
#include <sys/select.h>

#define PORT 8080
#define WEIGHED 0

int sockfd;

int check_connection() {
    printf("Checking connection\n");
    char buffer[1] = {0};  // dummy buffer
    if (send(sockfd, buffer, sizeof(buffer), MSG_NOSIGNAL) == -1) {
        printf("Sent correctly\n");
        return 0;
    } else {
        printf("Sent incorrectly\n");
        return 1;
    }
}

void send_weighed(int player, int team) {
    int message[3] = {WEIGHED, player, team};

    int sent = send(sockfd, message, sizeof(message), 0); 
    printf("Message sent: %d %d\n", player, team);
    printf("Message sent correctly: %i\n",sent);
}

int create_client(char *ip_address) {
    printf("Creating client\n\n\n\n");
    struct sockaddr_in servaddr; 

    // Create socket file descriptor 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
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
    } else if (rv == 0) {
        printf("connection timed out.\n");
    } else {
        // one or both of the descriptors have data
        int error;
        socklen_t len = sizeof (error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

        if (error == 0) {
            // socket is successfully connected
            fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) & ~O_NONBLOCK);
            send_weighed(0,5); //connect message
            return sockfd;
        } else {
            // error occurred, connection failed
            close(sockfd);
            perror("connection failed");
            exit(EXIT_FAILURE);
        }
    }
    return -1;
}

void close_client() {
	close(sockfd);
}
