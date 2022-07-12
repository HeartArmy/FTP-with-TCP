#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "clientHeader.h"

int main(int argc, char ** argv) {
    if (argc == NUM_INPUT && atoi(argv[2]) != PORT_NUM) {

        printf("run: ./FTPClient <ftp-server-ip-address> <ftp-server-port-number> or ./FTPClient\n");
        printf("Port number: %d\n", PORT_NUM);
        exit(-1);
    } else if (argc != 1 && argc != NUM_INPUT) {
        printf("run: ./FTPClient <ftp-server-ip-address> <ftp-server-port-number> \n or: ./FTPClient\n");
        printf("Port number: %d\n", PORT_NUM);
        exit(-1);
    }

    // socket is initialized
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 0;
    }

    // connect is used here
    struct sockaddr_in server_address;
    bzero( & server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(1025);

    if ((argc == NUM_INPUT) && (inet_pton(AF_INET, argv[1], & server_address.sin_addr) <= 0)) {
        printf("Check input IP Address\n");
        printf("run: ./FTPClient <ftp-server-ip-address> <ftp-server-port-number> \n or run: ./FTPClient\n");
        printf("Port number: %d\n", PORT_NUM);
        exit(-1);
    } else {
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    }

    if (connect(server_fd, (struct sockaddr * ) & server_address, sizeof(server_address)) < 0) {
        perror("connect");
        return 0;
    } else
        printf("220 Service ready for new user.\n");



    int data_port = 1027; //we hardcoded it client port here and lost marks 0.5
    // printf("LOOK here %d", getsockname()); //this doesnt work, 0 port is supposed to be automatically assigned by OS, turns out this doesnt work too

        

    // send/recv job here
    while (1) {
        printf("ftp> ");

        char buffer[BUFFER_SIZE];
        bzero( & buffer, sizeof(buffer));
        char command[BUFFER_SIZE], input[BUFFER_SIZE];
        bzero( & command, sizeof(command));
        bzero( & input, sizeof(input));

        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        sscanf(buffer, "%s %s", command, input);

        if (cmdRunner(buffer, command, input, server_fd, & data_port) < 0)
            break;
    }
    return 0;
}