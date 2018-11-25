#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define PORT 8888

#define TRUE 1
#define FALSE 0

int main(){
    int clientSocket, ret;
    struct sockaddr_in serverAddr;
    char buffer[1024];
    const char MARKER[] = "__MARKER__";
    char marker_buffer[sizeof(MARKER)];

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Client Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(ret < 0){
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Connected to Server.\n");

    while (TRUE) {
        printf("Enter message: ");
        scanf("%s", buffer);
        if (recv(clientSocket, marker_buffer, 10, 0) <= 0) {
            perror("Server unavailible");
            exit(1);
        }

        marker_buffer[sizeof(MARKER) - 1] = 0;
        if (strcmp(marker_buffer, MARKER) != 0) {
            continue;
        }
        char ack = 0;
        send(clientSocket, &ack, 1, 0);

        if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
            exit(1);
        }
        recv(clientSocket, &ack, 1, 0);

        if (!strcmp(buffer, ":exit")) {
            printf("Shutting down...\n");
            return 0;
        }

        recv(clientSocket, buffer, 1023, 0);
        send(clientSocket, "A", 1, 0);

        printf("Server message: %s\n", buffer);
    }

    return 0;
}
