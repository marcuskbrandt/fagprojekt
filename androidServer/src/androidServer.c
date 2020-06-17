#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include "androidServer.h"
#define MAX 64 
#define PORT 8080
#define SA struct sockaddr 
int sockfd, connfd, len; 
int connfd2;
struct sockaddr_in servaddr, cli;
int connected;


// PORT 8080 til den ene og port 8181 til den anden.
void startServer(){
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    //Reuse address on reconnect.
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

#ifdef SO_REUSEPORT
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
        perror("setsockopt(SO_REUSEPORT) failed");
#endif

    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli); 

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
    } else
        printf("Send thread acccept the client...\n");
    connected = 1;
}

void serverReadFunc(int connfd2){
    printf("--------Read Function -------\n");
    char buff[MAX];
    bzero(buff, MAX); 
    
    int valread;
    if(valread = read(connfd2, buff, sizeof(buff)) > 0){
        if(strcmp("EXIT",buff) != 0){
            printf("From AndroidAPP: %s\n", buff);
        } else {
            printf("AndroidAPP CLOSED: %s\n", buff);
            close(sockfd);
            close(connfd);
            connected = 0;
            startServer();
        }
    } 
}

void serverRead(void *threadID){
    long tid;
    tid = (long)threadID;
    printf("CREATED READ THREAD SUCCESFULLY! \n");
  
    printf("--------ReadThread -------\n");
    // Accept the data packet from client and verification
    while(1){
        while(connected){
        connfd2 = accept(sockfd, (SA*)&cli, &len); 
        if (connfd2 < 0) { 
        printf("server acccept failed...\n"); 
            } else
        printf("Read server acccept the client...\n");
        serverReadFunc(connfd2);
        close(connfd2);
    }
}
    pthread_exit(NULL);
}

void serverSendFunc(int connfd){
    printf("--------Send Function -------\n");
    char buff[MAX];
    bzero(buff, MAX); 
    int n;
    bzero(buff, sizeof(buff)); 
    printf("Enter the string : "); 
    n = 0; 
    while ((buff[n++] = getchar()) != '\n');

    int val = write(connfd, buff, sizeof(buff));
    printf("%d",val);
}

void serverSend(void *threadID){
    long tid;
    tid = (long)threadID;
    printf("CREATED SEND THREAD SUCCESFULLY! \n");
    while(1){
        while(connected){
        serverSendFunc(connfd);
        }
    }
    pthread_exit(NULL);
}

// Driver function 
int main() { 
pthread_t threads[2];

startServer();

int t1;
t1 = pthread_create(&threads[0], NULL, serverRead, (void *)0);
if (t1){
    printf("Unable to create read thread\n");
}

int t2;
t2 = pthread_create(&threads[1], NULL, serverSend, (void *)1);
if (t2){
    printf("Unable to create send thread\n");
}
pthread_exit(NULL);

} 
