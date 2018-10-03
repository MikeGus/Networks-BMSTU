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

void handleError(char* error) {
    fprintf(stderr, "%s\n", error);
    exit(-1);
}


int main() {
    struct sockaddr_in sockThis, sockOther;
    char buf[BUFLEN];

    int s = 0, sockLen = sizeof(sockOther);
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        handleError("Can't create socket!");
    }

    memset((char *) &sockThis, 0, sockLen);

    sockThis.sin_family = AF_INET;
    sockThis.sin_port = htons(PORT);
    sockThis.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, &sockThis, sockLen) == -1) {
        handleError("Bind error!");
    }

    while (1) {
        if (recvfrom(s, buf, BUFLEN, 0, &sockOther, &sockLen) == -1) {
            handleError("Recvfrom error!");
        }
        printf("Recieved message from %s: %d\nContent: %s\n",
               inet_ntoa(sockOther.sin_addr), ntohs(sockOther.sin_port), buf);
    }

    return 0;
}
