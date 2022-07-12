#define BUFFER_SIZE 2049
#define PATH_MAX 4096
#define NUM_USERS 1024
extern char * directories[];
extern char * users[];
extern char * passwords[];
extern char * pass_check[];

typedef struct args {
    int client_sd;
    char * input;
    char * path;
}
ARGS;

typedef struct serve_arg {
    int client_sd;
    char * command;
    char * input;
    char * path;
}
SERVE_ARG;

//these are helper functions for server side
int initiate_comm(int client_fd, fd_set * authen_fdset, fd_set * user_fdset, int num_users);
int allCmdHelper(char * path, char * input, char * command, int client_sd);
int portHelper(char * input, char * command, int server_sd);
void * cmdHelper(void * in_args);
void * dataTransferHelper(void * s_arg);
void * storHelper(void * in_arg);
void * retrHelper(void * client_sd_ptr);