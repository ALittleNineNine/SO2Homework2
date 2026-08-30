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
#define CHECK_INTERVAL 1

#define BUFFER_SIZE 1024

typedef struct {
    int client_socket; // canale per comunicare con il client
    int id_client; 
} ConnectionInfo;

// crea socket del server, lo configura e lo mette in ascolto, ritorna il fd del socket
int create_server();

/* ____________________ LOG ____________________ */

// crea directory log se non esistente, e file log iniziale
void create_file_log();

// ottiene timestamp attuale in una stringa leggibile
void get_current_time(char *buffer, size_t buffer_size);

// scrittura messaggio (dato o DISCONNECT) su log con timestamp e lock
void write_log(int id_client, const char *data);

// disconnect log
void disconnect_helper(int id_client);

// controllo dimensione log e eventualmente avvia rotazione
void check_log_size();

// rotazione log: archivia file log corrente e ne crea uno nuovo
void rotate_log();

/* ____________________ GESTIONE CLIENT ____________________ */

// gestire client (processo padre): accetta connessioni e forka per ogni client
void accept_client(int server_fd);

// gestire client (processo figlio): legge e scrive su log i messaggi di un client
void manage_client(ConnectionInfo info);

/* ____________________ GESTIONE SEGNALE ____________________ */

// gestire segnali: registra handler e avvia timer
void set_signal(int server_fd);

// gestisce i SIGPIPE, SIGINT, SIGALRM
void signal_handler(int sg);

/* ____________________ PULIZIA ____________________ */

// chiude server socket
void clean();

#endif



