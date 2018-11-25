#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAX_MSG_SIZE 0xFFFF
#define MAX_RESPONSE_SIZE 64
#define MAX_FILENAME_SIZE 64
#define PORT 3000


void start_chat(int sockfd) {
    char buff[MAX_MSG_SIZE];
    bzero(buff, sizeof(buff));

    for(int i = 0; i < 2; ++i) {
        printf("Filename: ");
        char filename[MAX_FILENAME_SIZE];
        scanf("%s", filename);
        int file = open(filename, 0);

        ssize_t fileSizeActual = read(file, buff, MAX_MSG_SIZE);

        write(sockfd, buff, fileSizeActual);
        bzero(buff, sizeof(buff));

        char response[MAX_RESPONSE_SIZE] = {0};
        read(sockfd, response, sizeof(response));

        printf("From Server: %s\nActual: %u\n", response, (unsigned) fileSizeActual);
    }
}


int main() {
    int sockfd = 0;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET,SOCK_STREAM,0);

    if (sockfd == -1) {
        printf("> Socket creation failed.\n");
        exit(0);
    } else {
        printf("> Socket successfully created.\n");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd,(struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("> Connection with the server failed.\n");
        exit(0);
    } else {
        printf("> Connected to the server.\n");
    }

    start_chat(sockfd);
    close(sockfd);
}
