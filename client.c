// librerie standard C
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <errno.h> 

// system programming
#include <unistd.h> 
#include <sys/types.h>

// Network
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>

#define SERVER_PORT 6666

// stampa un messaggio di errore e termina il programma
void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {

    // inizializzazioni
    int sockfd, n;
    int id, dato;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];   // conterrà il messaggio da inviare

    // controllo argomenti main
    if (argc != 2) {
        printf("%s <IP aggregatore>\n", argv[0]);
        exit(1);
    }

    // creazione socket TCP su IPv4
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Errore: impossibile creare il socket");
    }

    // ottenere e controllare IP aggregatore
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        error("Errore: IP aggregatore non valido");
    }

    // compilare indirizzo server
    memset(&serv_addr, 0, sizeof(serv_addr));  // azzerare
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(SERVER_PORT);

    // controllo connessione
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Errore: impossibile connettere al server");
    }

    printf("Connessione al server con successo\n");
    printf("Digitare coppie \"ID DATO\" per inviarle al server:\n");

    // ottenere, generare e scrivere il messaggio da inviare in ogni ciclo
    while (fscanf(stdin, "%d %d", &id, &dato) == 2) {

        // ottenere e generare il messaggio da inviare
        memset(buffer, 0, sizeof(buffer));     // azzerare
        snprintf(buffer, sizeof(buffer), "[%d, %d]\n", id, dato);

        // scrittura sul socket del messaggio da inviare
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            error("Errore: impossibile scrivere sul socket");
        }

    }

    char receive[128];
    memset(receive, 0, sizeof(receive));

    int bytes_received = read(sockfd, receive, sizeof(receive) - 1);
    if (bytes_received > 0) {
        receive[bytes_received] = '\0';
    }
    // chiusura socket
    close(sockfd);

    return 0;

}