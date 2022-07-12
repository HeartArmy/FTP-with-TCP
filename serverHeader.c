#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "serverHeader.h"

int initiate_comm(int client_fd, fd_set * authen_fdset, fd_set * user_fdset, int num_users) {
    char message[BUFFER_SIZE];
    bzero( & message, sizeof(message));
    char response[BUFFER_SIZE];
    bzero( & response, BUFFER_SIZE);

    if (recv(client_fd, message, sizeof(message), 0) < 0) {
        perror("recv");
        return -1;
    }

    if (strcmp(message, "QUIT") == 0) {
        strcpy(response, "221 Service closing control connection.");
        send(client_fd, response, strlen(response), 0);
        printf("***Client fd = %d disconnected \n", client_fd);
        close(client_fd);
        return -1;
    }

    char command[BUFFER_SIZE], input[BUFFER_SIZE];
    bzero( & command, sizeof(command));
    bzero( & input, sizeof(input));
    sscanf(message, "%s %s", command, input);

    //User authentication

    if (FD_ISSET(client_fd, authen_fdset)) {
        if (strcmp(command, "USER") == 0 || strcmp(command, "PASS") == 0) {
            strcpy(response, "230 User logged in, proceed.");
            send(client_fd, response, strlen(response), 0);
        } else if (strcmp(command, "PORT") == 0) {

            strcpy(response, "ready");
            send(client_fd, response, strlen(response), 0);

            //1. socket
            int server_sd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_sd < 0) {
                perror("Socket Failed: ");
                return -1;
            }

            int client_sd = portHelper(input, command, server_sd);

            if (client_sd) {
                strcpy(response, "200 PORT command successful. \n150 File status okay; about to open data connection.");
                send(client_fd, response, strlen(response), 0);

                char message[BUFFER_SIZE];
                bzero( & message, sizeof(message));
                char response[BUFFER_SIZE];
                bzero( & response, BUFFER_SIZE);

                if (recv(client_fd, message, sizeof(message), 0) < 0) {
                    perror("recv");
                    return -1;
                }

                char command[BUFFER_SIZE], input[BUFFER_SIZE];
                bzero( & command, sizeof(command));
                bzero( & input, sizeof(input));
                sscanf(message, "%s %s", command, input);

                if (strcmp(command, "RETR") == 0) {
                    size_t length = strlen(directories[client_fd]) + strlen(input) + 2;
                    char * path = (char * ) malloc(length * sizeof(char));
                    strcpy(path, directories[client_fd]);
                    strcat(path, "/");
                    strcat(path, input);
                    FILE * fptr = fopen(path, "r");
                    if (!fptr) {
                        strcpy(response, "550 No such file or directory.");
                        send(client_fd, response, strlen(response), 0);
                    } else {
                        strcpy(response, "150 File status okay; about to open data connection.");
                        send(client_fd, response, strlen(response), 0);
                        allCmdHelper(directories[client_fd], input, command, client_sd);
                    }
                } else if (strcmp(command, "STOR") == 0) {
                    strcpy(response, "ready");
                    send(client_fd, response, strlen(response), 0);
                    allCmdHelper(directories[client_fd], input, command, client_sd);
                } else if (strcmp(command, "LIST") == 0) {
                    strcpy(response, "ls_pwd");
                    send(client_fd, response, strlen(response), 0);

                    bzero( & command, sizeof(command));
                    strcpy(command, "ls");

                    allCmdHelper(directories[client_fd], input, command, client_sd);

                }

            } else {
                strcpy(response, "PORT command is NOT successful");
                send(client_fd, response, strlen(response), 0);
            }

            close(server_sd);
        } else if (strcmp(command, "CWD") == 0 || strcmp(command, "PWD") == 0) {
            {
                if (strcmp(command, "CWD") != 0) {
                    sprintf(response, "Server current directory: %s", directories[client_fd]);
                    send(client_fd, response, strlen(response), 0);

                } else {

                    chdir(directories[client_fd]);

                    if (chdir(input) == -1) {
                        strcpy(response, "Error: No such file or directory");
                        send(client_fd, response, strlen(response), 0);
                        printf(">>>Failed to excute cd command!\n");
                    } else {
                        printf("-->Executing cd command....");

                        char buf[BUFFER_SIZE];
                        bzero(buf, sizeof(buf));
                        FILE * in ;
                        if (( in = popen("pwd", "r")) != NULL) {
                            fgets(buf, BUFFER_SIZE, in );
                        }
                        buf[strcspn(buf, "\n")] = '\0';

                        char new_dir[BUFFER_SIZE];
                        bzero(new_dir, sizeof(new_dir));

                        strcpy(response, "200 directory changed to");

                        sprintf(new_dir, "%s %s", response, buf);
                        send(client_fd, new_dir, strlen(new_dir), 0);

                        int len = strlen(buf) + 1;
                        char * str = (char * ) malloc(len * sizeof(char));
                        directories[client_fd] = str;
                        strcpy(str, buf);
                        printf("Done %s\n", buf);
                    }
                    chdir(directories[0]);
                }
            }

        } else {
            strcpy(response, "202 Command not implemented.");
            send(client_fd, response, strlen(response), 0);
        }
    }
    //user authentication
    else if (strcmp(command, "USER") == 0) {
        for (int i = 0; i < num_users; i++) {
            if (strcmp(input, users[i]) == 0) {

                FD_SET(client_fd, user_fdset);

                strcpy(response, "331 Username OK, need password.");
                send(client_fd, response, strlen(response), 0);

                int len = strlen(passwords[i]) + 1;
                char * str = (char * ) malloc(len * sizeof(char));
                pass_check[client_fd] = str;
                strcpy(str, passwords[i]);

                return 0;
            }
        }

        strcpy(response, "530 Not logged in.");
        send(client_fd, response, strlen(response), 0);
        FD_CLR(client_fd, user_fdset);
    } else if (strcmp(command, "PASS") == 0) {
        if (FD_ISSET(client_fd, user_fdset)) {
            if (strcmp(input, pass_check[client_fd]) == 0) {
                FD_SET(client_fd, authen_fdset);
                strcpy(response, "230 User logged in, proceed.");
                send(client_fd, response, strlen(response), 0);
            } else {
                strcpy(response, "530 Not logged in.");
                send(client_fd, response, strlen(response), 0);
            }
        } else {
            strcpy(response, "530 Not logged in.");
            send(client_fd, response, strlen(response), 0);
        }
    } else {
        strcpy(response, "530 Not logged in.");
        send(client_fd, response, strlen(response), 0);
    }

    return 0;
}

int allCmdHelper(char * path, char * input, char * command, int client_sd) {

    SERVE_ARG * s_arg = malloc(sizeof(SERVE_ARG));
    s_arg -> client_sd = client_sd;
    int len1 = strlen(path) + 1;
    char * str1 = (char * ) malloc(len1 * sizeof(char));
    s_arg -> path = str1;
    strcpy(str1, path);

    int len2 = strlen(input) + 1;
    char * str2 = (char * ) malloc(len2 * sizeof(char));
    s_arg -> input = str2;
    strcpy(str2, input);

    int len3 = strlen(command) + 1;
    char * str3 = (char * ) malloc(len3 * sizeof(char));
    s_arg -> command = str3;
    strcpy(str3, command);
    //multithreading
    pthread_t thread_id;
    pthread_create( & thread_id, NULL, dataTransferHelper, s_arg);

    return 0;
}

void * dataTransferHelper(void * s_arg) {
    SERVE_ARG * r_arg = (SERVE_ARG * ) s_arg;
    int client_sd = r_arg -> client_sd;
    char * input = r_arg -> input;
    char * path = r_arg -> path;
    char * command = r_arg -> command;

    chdir(path);
    if (strcmp(command, "RETR") == 0) {
        retrHelper( & client_sd);
    } else if (strcmp(command, "STOR") == 0) {

        ARGS * arg = malloc(sizeof(ARGS));
        arg -> client_sd = client_sd;
        int len = strlen(input) + 1;
        char * str = (char * ) malloc(len * sizeof(char));
        arg -> input = str;
        strcpy(str, input);

        storHelper(arg);
    } else if (strcmp(command, "ls") == 0 || strcmp(command, "pwd") == 0) {
        ARGS * arg = (ARGS * ) malloc(sizeof(ARGS));
        arg -> client_sd = client_sd;

        int len2 = strlen(path) + 1;
        char * str2 = (char * ) malloc(len2 * sizeof(char));
        arg -> path = str2;
        strcpy(str2, path);

        int len = strlen(command) + 1;
        char * str = (char * ) malloc(len * sizeof(char));
        arg -> input = str;
        strcpy(str, command);

        cmdHelper(arg);
    }
    if (strcmp(command, "CWD") == 0) {
        ARGS * arg = (ARGS * ) malloc(sizeof(ARGS));
        arg -> client_sd = client_sd;
        int len = strlen(input) + 1;
        char * str = (char * ) malloc(len * sizeof(char));
        arg -> input = str;
        strcpy(str, input);
    }

    chdir(directories[0]);

    return NULL;
}
//helping with stor command
void * storHelper(void * in_arg) {
    ARGS * arg = in_arg;
    int client_sd = arg -> client_sd;
    char * filename = arg -> input;

    printf("-->Getting File: %s ...\n", filename);
    char buffer[1500];
    bzero(buffer, sizeof(buffer));
    int bytes;
    long long int size = 0;
    FILE * fptr;
    if (!(fptr = fopen(filename, "w")))
        perror("Cant create file");
    else {
        do {
            bytes = recv(client_sd, buffer, sizeof(buffer), 0);
            if (bytes > 0) {
                size += bytes;
                fwrite(buffer, bytes, 1, fptr);
            }
        } while (bytes > 0);
        fclose(fptr);
        printf(">>>Done. Bytes received: %lld\n", size);
    }
    close(client_sd);

    return NULL;
}

void * retrHelper(void * client_sd_ptr) {
    int client_sd = * (int * )(client_sd_ptr);
    char filename[50];
    bzero(filename, sizeof(filename));

    if (recv(client_sd, filename, sizeof(filename) - 1, 0) < 0)
        perror("Recv");
    else {
        printf("-->Sending File: %s ...\n", filename);
        FILE * fptr = fopen(filename, "r");
        if (!fptr) {
            perror("Cant open the file");
        } else {
            fseek(fptr, 0, SEEK_END); //move the fptr to the end of the file
            long long int file_size = ftell(fptr); //return the current position of fptr (number of bytes from the begining of the file)
            rewind(fptr); //move the fptr back to the begining of the file

            int bytes = 0;
            char buffer[1500];

            for (int i = 0; i < file_size / 1500; i++) {
                fread(buffer, sizeof(buffer), 1, fptr);
                bytes += write(client_sd, buffer, sizeof(buffer));
            }
            fread(buffer, file_size % 1500, 1, fptr);
            bytes += write(client_sd, buffer, file_size % 1500);

            printf("226 Transfer completed.");
        }

        fclose(fptr);
        close(client_sd);
    }
    return NULL;
}

void * cmdHelper(void * in_arg) {
    ARGS * arg = (ARGS * ) in_arg;
    int client_sd = arg -> client_sd;
    char * command = arg -> input;

    printf("-->Executing %s command...Done \n", command);
    char sendline[BUFFER_SIZE + 1];
    bzero(sendline, (int) sizeof(sendline));
    FILE * in ;

    if (!( in = popen(command, "r"))) {
        sprintf(sendline, "wrong command usage!\n"); //error handling
        write(client_sd, sendline, strlen(sendline));
        close(client_sd);
        return NULL;
    }

    sprintf(sendline, " "); //had to do this to make sure we are not printing errors twice
    send(client_sd, sendline, strlen(sendline), 0);
    bzero(sendline, (int) sizeof(sendline));

    sprintf(sendline, " "); 

    write(client_sd, sendline, strlen(sendline));
    bzero(sendline, (int) sizeof(sendline));
    while (fgets(sendline, BUFFER_SIZE, in ) != NULL) {
        write(client_sd, sendline, strlen(sendline));
        bzero(sendline, (int) sizeof(sendline));
    }

    close(client_sd);
    return NULL;
}

int portHelper(char * input, char * command, int server_sd) {

    int client_ip[4], clientPort, port_parts[2];
    char clientIP[50];
    sscanf(input, "%d,%d,%d,%d,%d,%d", & client_ip[0], & client_ip[1], & client_ip[2], & client_ip[3], & port_parts[0], & port_parts[1]);
    sprintf(clientIP, "%d.%d.%d.%d", client_ip[0], client_ip[1], client_ip[2], client_ip[3]);
    clientPort = (port_parts[0] * 256) + port_parts[1];

    // Prepare Socket info
    struct sockaddr_in server_address; // this structure is to have IP address and port
    memset( & server_address, 0, sizeof(server_address)); //Initialize or fill the server_address to 0
    server_address.sin_family = AF_INET; //address of family
    server_address.sin_port = htons(clientPort); //has the port
    server_address.sin_addr.s_addr = inet_addr(clientIP);

    if (setsockopt(server_sd, SOL_SOCKET, SO_REUSEADDR, & (int) {
            1
        }, sizeof(int)) < 0) {
        perror("Setsockopt:");
        return -1;
    }
    //2. bind
    if (bind(server_sd, (struct sockaddr * ) & server_address, sizeof(server_address)) < 0) {
        perror("Bind failed..:");
        return -1;
    }
    //3. listen

    if (listen(server_sd, 5) < 0) {
        perror("Listen Error:");
        return -1;
    }

    int client_sd = accept(server_sd, NULL, NULL);

    if (client_sd < 0) {
        perror("Connection failed..!");

        return -1;
    } else {

        return client_sd;
    }

}