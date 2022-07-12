#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include "serverHeader.h"

char * directories[1024];
char * users[NUM_USERS];
char * passwords[NUM_USERS];
char * pass_check[1024];

int main() {
    //user authentication and reading from txt file
    FILE * file = fopen("users.txt", "r");
    char line[256];
    char user[128];
    char password[128];

    int num_users = 0;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %s", user, password);

        int len = strlen(user) + 1;
        char * str = (char * ) malloc(len * sizeof(char));
        users[num_users] = str;
        strcpy(str, user);

        int len2 = strlen(password) + 1;
        char * str2 = (char * ) malloc(len2 * sizeof(char));
        passwords[num_users] = str2;
        strcpy(str2, password);

        num_users++;
    }

    fclose(file);

    //1. socket();
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, & (int) {
            1
        }, sizeof(int)) < 0) {
        perror("setsock");
        return -1;
    }
    // bind () here
    struct sockaddr_in server_address;
    bzero( & server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(1025);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_fd, (struct sockaddr * ) & server_address, sizeof(server_address)) < 0) {
        perror("bind");
        return -1;
    }

    // listen() here

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        return -1;
    }
    fd_set full_fdset, ready_fdset, authen_fdset, user_fdset;
    FD_ZERO( & authen_fdset);
    FD_ZERO( & full_fdset);
    FD_ZERO( & user_fdset);
    FD_SET(server_fd, & full_fdset);
    FD_SET(0, & full_fdset);
    int max_fd = server_fd;

    char server_path[PATH_MAX];
    if (getcwd(server_path, sizeof(server_path)) != NULL) {
        int len = strlen(server_path) + 1;
        char * str = (char * ) malloc(len * sizeof(char));
        directories[server_fd] = str;
        directories[0] = str;
        strcpy(str, server_path);
    } else {
        perror("getcwd() error");
        return 1;
    }
    //4. accept()
    printf("Listening....\n");
    while (1) {

        // connect multiple clients with select
        ready_fdset = full_fdset;
        if (select(max_fd + 1, & ready_fdset, NULL, NULL, NULL) < 0) {
            perror("select");
            return -1;
        }

        for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, & ready_fdset)) {
                if (fd == server_fd) {
                    int new_fd = accept(server_fd, NULL, NULL);
                    printf("***Client fd = %d has established a connection\n", new_fd);
                    directories[new_fd] = server_path;
                    FD_SET(new_fd, & full_fdset);
                    if (new_fd > max_fd)
                        max_fd = new_fd;
                } else if (fd == 0) {
                    char buffer[BUFFER_SIZE];
                    bzero(buffer, sizeof(buffer));
                    read(fd, (void * ) & buffer, 10);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    if (strcmp(buffer, "quit") == 0) {
                        printf("Terminated\n");

                        //4. close
                        close(server_fd);
                        return 0;
                    }
                } else {
                    if (initiate_comm(fd, & authen_fdset, & user_fdset, num_users) == -1) {
                        FD_CLR(fd, & full_fdset);
                        FD_CLR(fd, & authen_fdset);
                        if (max_fd == fd) {
                            for (int i = max_fd - 1; i >= 0; i--)
                                if (FD_ISSET(i, & full_fdset)) {
                                    max_fd = i;
                                    break;
                                }
                        }
                    }
                }
            }
        }
    }

    // we close connection
    close(server_fd);

    return 0;
}