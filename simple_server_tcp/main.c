#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFLEN 100000
#define PORT 5001

void handleError(char* error) {
    fprintf(stderr, "%s\n", error);
    exit(-1);
}


int main() {
    struct sockaddr_in sockThis, sockOther;

    char* buf = calloc(sizeof(char), BUFLEN);
    if (buf == NULL) {
        handleError("Can't allocate memory");
    }

    int s = 0, sockLen = sizeof(sockOther);
    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        handleError("Can't create socket!");
    }

    memset((char *) &sockThis, 0, sockLen);

    sockThis.sin_family = AF_INET;
    sockThis.sin_port = htons(PORT);
    sockThis.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, &sockThis, sockLen) == -1) {
        handleError("Bind error!");
    }

    listen(s, 3);

    int clS = accept(s, (struct sockaddr *)&sockOther, (socklen_t*)&sockLen);
    if (clS < 0) {
       handleError("Accept error");
    }

    while (1) {

        ssize_t size = recv(clS, buf, BUFLEN - 1, 0);
        if (size == 0) {
            handleError("Recv error");
        }

        printf("Recieved message from %s: %d\nContent: %s\n",
               inet_ntoa(sockOther.sin_addr), ntohs(sockOther.sin_port), buf);

        for (ssize_t i = 0; i < size; ++i) {
            buf[i] = 0;
        }

    }

    return 0;
}
