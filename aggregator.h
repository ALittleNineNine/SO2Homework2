#ifndef AGGREGATOR_H
#define AGGREGATOR_H

// librerie standard C
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <errno.h> 

// system programming
#include <unistd.h> 
#include <fcntl.h>
#include <signal.h> 
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/wait.h>

// Network
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>

#include <time.h> 

#define DEFAULT_PORT 6666
#define LOG_FILE_PATH "./logs/logfile.txt"
#define LOG_DIR "./logs/"
#define MAX_LOG_SIZE 200 

#define MAX_CLIENTS_WAITING 10 
#define CHECK_INTERVAL 5

#define BUFFER_SIZE 1024

typedef struct {
    int client_socket; // canale per comunicare con figlio
    int id_client; 
} ConnectionInfo;

// crea server
int create_server();

// log
void create_file_log();
void get_current_time(char *buffer, size_t buffer_size);
void write_log(int id_client, const char *data);
void disconnect_helper(int id_client);
void check_log_size();
void rotate_log();

// gestione client
void accept_client(int server_fd);
void manage_client(ConnectionInfo info);

// gestione segnale
void set_signal(int server_fd);
void signal_handler(int sg);

// pulizia
void clean();

#endif
