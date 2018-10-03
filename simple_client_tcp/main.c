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
#define SERVER_IP "127.0.0.1"

void handleError(char* error) {
    fprintf(stderr, "%s\n", error);
    exit(-1);
}

int readLine(char* line) {
    char ch = 0;
    int count = 0;
    while ((ch = getchar()) != '\n' && ch != 0) {
        *line++ = ch;
        count++;
    }
    *line = 0;
    return count;
}

int main() {
    struct sockaddr_in sock;
    int s = 0, sockLen = sizeof(sock);

    char* buf = calloc(sizeof(char), BUFLEN);
    if (buf == NULL) {
        handleError("Can't allocate memory");
    }


    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        handleError("Can't create socket!");
    }

    memset((char *) &sock, 0, sizeof(sockLen));

    sock.sin_family = AF_INET;
    sock.sin_port = htons(PORT);
    if (inet_aton(SERVER_IP, &sock.sin_addr) == 0) {
        handleError("Error in other server ip read!");
    }

    if (connect(s, (struct sockaddr *)&sock, sockLen) < 0) {
        handleError("Can't connect");
    }

    int correct = 1;
    while (correct) {
        printf("Enter message for server: ");

        correct = readLine(buf);

        if (send(s, buf, strlen(buf), 0) < 0) {
            handleError("Can't send");
        }
    }

    close(s);

    return 0;
}
