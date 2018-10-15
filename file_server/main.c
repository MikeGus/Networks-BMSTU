#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_MSG_SIZE 0xFFFF
#define MAX_RESPONSE_SIZE 64
#define PORT 3000

void start_chat(int sockfd) {
    char buff[MAX_MSG_SIZE] = {0};

    for (int i = 0; i < 2; ++i) {
        ssize_t fileSize = read(sockfd, buff, sizeof(buff));

        bzero(buff, MAX_MSG_SIZE);

        char response[MAX_RESPONSE_SIZE];
        sprintf(response, "Size of file: %u\n", (unsigned) fileSize);

        write(sockfd, response, sizeof(response));
    }
}


int main() {
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("> Socket creation failed.\n");
        exit(0);
    } else {
        printf("> Socket successfully created.\n");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
        printf("> Socket bind failed.\n");
        exit(0);
    } else {
        printf("> Socket successfully binded.\n");
    }

    if ((listen(sockfd, 5)) != 0) {
        printf("> Listen failed.\n");
        exit(0);
    } else {
        printf("> Server listening.\n");
    }

    len = sizeof(cli);
    connfd = accept(sockfd, (struct sockaddr *)&cli, (socklen_t*)&len);

    if (connfd < 0) {
        printf("> Server acccept failed.\n");
        exit(0);
    } else {
        printf("> Server acccept the client.\n");
    }

    start_chat(connfd);
    close(sockfd);
}
