//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux
#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>


#define TRUE   1
#define FALSE  0
#define PORT 8888


#define MAX_CLIENTS 5

static int master_socket;

void mac_eth0(int s, char* MAC_str)
{
    #define HWADDR_len 6
    int i;
    struct ifreq ifr;
    strcpy(ifr.ifr_name, "wlp4s0");
    ioctl(s, SIOCGIFHWADDR, &ifr);
    for (i=0; i<HWADDR_len; i++)
        sprintf(&MAC_str[i*2],"%02X",((char*)ifr.ifr_hwaddr.sa_data)[i]);
    MAC_str[12]='\0';
}


void handle(int sig) {
    if (sig == SIGINT) {
        close(master_socket);
        exit(0);
    }
}


void write_clients(int* client_socket) {
    system("> clients");
    system("echo \"SOCKET\t\t\tFD\t\t\tMAC\" >> clients");
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        int sd = client_socket[i];
        if (sd > 0) {
            struct sockaddr_in address;
            socklen_t addrlen;
            getpeername(sd, (struct sockaddr*) &address, (socklen_t*) &addrlen);
            char MAC[13];
            mac_eth0(sd, MAC);
            char str[64];
            sprintf(str, "echo \"%s:%d\t\t%d\t\t\t%s\" >> clients", inet_ntoa(address.sin_addr), ntohs(address.sin_port), sd, MAC);
            system(str);
        }
    }
}

int main()
{
    int pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        while (TRUE) {
            char cmd[20];
            scanf("%9s", cmd);
            if (!strcmp(cmd, ":all")) {
                system("route -n");
            } else if (!strcmp(cmd, ":clients")) {
                system("cat clients");
            } else if (cmd[0] == ':') {
                char systemcmd[64];
                strcpy(systemcmd, "route -n | grep ");
                strcat(systemcmd, cmd + 1);
                system(systemcmd);
            }
        }
    }
    int opt = TRUE;
    int addrlen, new_socket, client_socket[MAX_CLIENTS], max_clients = MAX_CLIENTS, activity, i, sd;
    ssize_t valread;
    int max_sd;
    struct sockaddr_in address;

    int current_queue = 0;

    char buffer[1024];
    const char MARKER[] = "__MARKER__";

    //set of socket descriptors
    fd_set readfds, writefds;


    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }

    //create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle);
    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr*) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while (TRUE) {
        //clear the socket set
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for (i = 0 ; i < max_clients; i++) {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if (sd > 0) {
                FD_SET(sd, &readfds);
                FD_SET(sd, &writefds);
            }

            //highest file descriptor number, need it for the select function
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select(max_sd + 1, &readfds, &writefds, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            char MAC[13];
            mac_eth0(new_socket, MAC);

            //inform user of socket number - used in send and receive commands
            printf("Client connected:\t%s:%d\tfd=%d\tMAC=%s\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port),
                   new_socket, MAC);
            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++) {
                //if position is empty
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    write_clients(client_socket);
                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        int counter = 0;
        int queue_start = current_queue;

        int alive_clients = 0;
        for (int j = 0; j < max_clients; ++j) {
            if (client_socket[j] > 0) {
                ++alive_clients;
            }
        }

        while (counter < alive_clients) {
            i = (queue_start + counter) % max_clients;
            sd = client_socket[i];


            if (FD_ISSET(sd, &writefds)) {
                char ack = 0;
                send(sd, MARKER, sizeof(MARKER) - 1, 0);
                recv(sd, &ack, 1, 0);

                valread = recv(sd, buffer, sizeof(buffer) - 1, 0);
                buffer[valread] = 0;
                send(sd, "A", 1, 0);

                if (!strcmp(buffer, ":exit")) {
                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    char MAC[13];
                    mac_eth0(sd, MAC);

                    //inform user of socket number - used in send and receive commands
                    printf("Client disconnected:\t%s:%d\tfd=%d\tMAC=%s\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port),
                           sd, MAC);
                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                    write_clients(client_socket);
                } else {
                    send(sd, buffer, strlen(buffer), 0);
                    recv(sd, &ack, 1, 0);
                }

                ++current_queue;
                current_queue %= alive_clients;
                break;
            }

            ++counter;
        }
    }

    return 0;
}
