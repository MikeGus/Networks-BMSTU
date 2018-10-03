#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFLEN 512
#define PORT 5000
#define SERVER_IP "127.0.0.1"

void handleError(char* error) {
    fprintf(stderr, "%s\n", error);
    exit(-1);
}

int main() {
    struct sockaddr_in sock;
    int s = 0, sockLen = sizeof(sock);
    char buf[BUFLEN];

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        handleError("Can't create socket!");
    }

    memset((char *) &sock, 0, sizeof(sockLen));

    sock.sin_family = AF_INET;
    sock.sin_port = htons(PORT);
    if (inet_aton(SERVER_IP, &sock.sin_addr) == 0) {
        handleError("Error in other server ip read!");
    }

    int correct = 1;
    while (correct) {
        printf("Enter message for server: ");
        correct = scanf("%s", buf);
        if (sendto(s, buf, BUFLEN, 0, &sock, sockLen) == -1) {
            handleError("Error sendto()!");
        }
    }

    close(s);

    return 0;
}
