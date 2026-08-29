#include "aggregator.h"

static int close_server_fd = -1; // fd del socket del server 
static volatile sig_atomic_t server_running = 1; // indica se server è in esecuzione 
static volatile sig_atomic_t current_id_client = -1; 

static volatile sig_atomic_t sigpipe_on = 0;
static volatile sig_atomic_t sigalrm_on = 0;

// crea socket del server, lo configura e lo mette in ascolto
int create_server() {
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("socket");
        return -1;
    }

    // SO_REUSEADDR per riuso rapido IP:PORT (riusare subito la porta dopo chiusura)
    int reuse_addr = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) < 0) {
        perror("setsockopt");
        close(sd);
        return -1;
    }

    // configurazione indirizzo: struttura con indirizzo su cui il server sarà raggiungibile
    struct sockaddr_in serv_addr; // indirizzo (IP + porta) del server a cui connettersi
    memset(&serv_addr, 0, sizeof(serv_addr)); // azzerra memoria
    serv_addr.sin_family = AF_INET; // famiglia di indirizzi: IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY; // accetta connessioni su qls interfaccia di rete
    serv_addr.sin_port = htons(DEFAULT_PORT); // porta di ascolto

    // bind: associa socket a indirizzo/porta
    if (bind(sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        close(sd);
        return -1;
    }

    // listen: metto socket in ascolto
    if (listen(sd, MAX_CLIENTS_WAITING) < 0) {
        perror("listen");
        close(sd);
        return -1;
    }
    
    printf("Server in ascolto sulla porta %d\n", DEFAULT_PORT);
    return sd; // return fd del socket
}

// crea directory se non esistente
void create_file_log() {
    if (mkdir("./logs/", 0755) == 0) {
        printf("Directory %s creata\n", LOG_DIR);
    } else if (errno == EEXIST) {
        printf("Directory %s esistente\n", LOG_DIR);
    } else {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }

    // crea/apre file log in modalità append
    FILE *fp = fopen(LOG_FILE_PATH, "a");
    if (!fp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fclose(fp);
    printf("File log: %s\n", LOG_FILE_PATH);
}

// ottieni timestamp attuale
void get_current_time(char *buffer, size_t buffer_size) {
    time_t now = time(NULL);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", localtime(&now)); // scrive nel buffer usando ora del sistema
}

// lock con fcntl
static int lock_on(int fd) {
    struct flock lock; // struttura per lock
    lock.l_type = F_WRLCK; // lock in scrittura 
    lock.l_whence = SEEK_SET; // inizio file
    lock.l_start = 0; 
    lock.l_len = 0; // tutto il file

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl lock");
        return -1;
    }
    return 0; // lock on con successo
    
}

// lock su un fd
static int lock_off(int fd) {
    struct flock lock; // struttura per lock
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("fcntl unlock");
        return -1;
    }
    return 0; // lock off con successo
}

// scrittura messaggio su log con timestamp e lock
void write_log(int id_client, const char *data) {
    char timestamp[64];
    get_current_time(timestamp, sizeof(timestamp));

    // gestire DISCONNECT
    if (strcmp(data, "DISCONNECT") == 0) {
        // apri file in modalità append e se non esiste viene creato
        int fd = open(LOG_FILE_PATH, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (fd < 0) {
            perror("open");
            return;
        }

        // attiva lock
        if (lock_on(fd) == 0) {
            char entry[256];
            // [TIMESTAMP, ID_CLIENT, DISCONNECT]
            snprintf(entry, sizeof(entry), "[%s, %d, DISCONNECT]\n", timestamp, id_client);
            write(fd, entry, strlen(entry)); // scrivi nel file
            lock_off(fd);
        }
        close(fd);
        return;
    }

    // gestire messaggio normale
    int id, dato;
    // prendere ID e DATO dal formato [ID, DATO]
    if (sscanf(data, "[%d, %d]", &id, &dato) != 2) {
        fprintf(stderr, "Messaggio non valido: %s", data);
        return;
    }

    // apri file in modalità append
    int fd = open(LOG_FILE_PATH, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }

    // attiva lock
    if (lock_on(fd) == 0) {
            char entry[256];
            // [TIMESTAMP, ID, DATO]
            snprintf(entry, sizeof(entry), "[%s, %d, %d]\n", timestamp, id, dato);
            write(fd, entry, strlen(entry)); // scrivi nel file
            lock_off(fd);
        }
        close(fd);
}

// disconnect log 
void disconnect_helper(int id_client) {
    write_log(id_client, "DISCONNECT");
}

// controllo dimensione log
void check_log_size() {
    struct stat st; // struttura per info del file
    if (stat(LOG_FILE_PATH, &st) == 0) { // ottieni info file
        if (st.st_size >= MAX_LOG_SIZE) { // se supera dimensione massima
            printf("Dimensione log è %ld e supera il limite %d bytes\n", st.st_size, MAX_LOG_SIZE);
            rotate_log(); // ruota log
        }
    }
}

// rotazione log: archivia file log corrente e ne crea nuovo
void rotate_log() {
    char archive[512]; // nome file archiviato
    char timestamp[64];
    get_current_time(timestamp, sizeof(timestamp));
    // crea nome archivio
    snprintf(archive, sizeof(archive), "%s_%s.log", LOG_FILE_PATH, timestamp);

    // rinomina file corrente in archivio e archivia
    if (rename(LOG_FILE_PATH, archive) == 0) {
        printf("Log archiviato: %s\n", archive);
        create_file_log(); // crea nuovo file log
    } else {
        perror("rename");
    }
}

// gestire client (processo padre)
void accept_client(int server_fd) {
    struct sockaddr_in client_addr; // indirizzo client
    socklen_t len_addr = sizeof(client_addr);
    int count_client = 0; // contatore per ID client

    while (server_running) { // finché server in esecuzione
        while (waitpid(-1, NULL, WNOHANG) > 0); // ripulire processi figli terminati
        // accetta nuova connessione
        int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &len_addr);

        if (client_fd < 0) {
            if (errno == EINTR) {
                // segnale ha interrotto accept: gestisce segnale se ricevuto
                if (sigalrm_on) {
                sigalrm_on = 0;
                check_log_size();
                alarm(CHECK_INTERVAL);
                }
                continue; // ricomincia ciclo
            }
            if (!server_running) break; // esce pulito se server in fase spegnimento
            perror("accept");
            break;
        }

        // se accept ok controllare se allarme era scattato durante attesa
        if (sigalrm_on) {
                sigalrm_on = 0;
                check_log_size();
                alarm(CHECK_INTERVAL);
        }
        
        // crea struttura ConnectionInfo (info client)
        ConnectionInfo *info = malloc(sizeof(ConnectionInfo));
        if (!info) {
            perror("malloc");
            close(client_fd);
            continue;
        }
        info->client_socket = client_fd;
        info->id_client = ++count_client; // ID che incrementa

        printf("Client %d connesso\n", info->id_client);

        // crea processo figlio per gestire client corrente
        pid_t pid = fork();
        if (pid == 0) {
            close(server_fd); // figlio non usa socket in ascolto
            signal(SIGINT, SIG_IGN);    // processo figlio ignora SIGINT
            manage_client(*info); 
            free(info);
            exit(EXIT_SUCCESS); // figlio termina 
        } else if (pid > 0) {
            // processo padre
            close(client_fd); // padre chiude copia locale della socket del client
            free(info);
        } else {
            perror("fork");
            close(client_fd);
            free(info);
        }
    }
}

// gestire client (processo figlio)
void manage_client(ConnectionInfo info) {
    current_id_client = info.id_client; // salva ID per signal handler
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while(1) {
        // gestire segnale SIGPIPE
        if (sigpipe_on) {
            sigpipe_on = 0;
            int id_client = current_id_client;
            if (id_client > 0) {
                write_log(id_client, "DISCONNECT"); // scrivere DISCONNECT su log
                printf("Segnale SIGPIPE - Client %d disconnesso all'improvviso\n", id_client);
            }
            break; // esci da loop
        }

        // riceve dati client
        bytes_read = read(info.client_socket, buffer, BUFFER_SIZE - 1);

        if (bytes_read > 0) { // dati ricevuti
            buffer[bytes_read] = '\0'; // termina stringa
            printf("Client %d: %s", info.id_client, buffer);
            fflush(stdout); // forza stampa immediata
            // scrivi log
            write_log(info.id_client, buffer);
        } else if (bytes_read == 0) {
            // client disconnesso correttamente
            printf("Client %d disconnesso\n", info.id_client);
            disconnect_helper(info.id_client); // scrivi DISCONNECT su log
            break;
        } else {
            if (errno == EINTR) continue; // interrotto da segnale riprova
            perror("recv");
            disconnect_helper(info.id_client);
            break;
        }
    }
    current_id_client = -1; // reset id
    close(info.client_socket); // chiudi socket client
}

// gestire segnali
void set_signal(int server_fd) {
    close_server_fd = server_fd; // salva fd per signal handler

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGPIPE, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    alarm(CHECK_INTERVAL); // timer per SIGALRM
}

void signal_handler(int sg) {
    if (sg == SIGPIPE) {
        // SIGPIPE: client disconnesso all'improvviso durante scrittura
        sigpipe_on = 1; // gestito in manage_client()
    } else if (sg == SIGINT) {
        // SIGINT: client termina in modo controllato (ctrl + C)
        printf("Segnale SIGINT - Terminazione controllata\n");
        server_running = 0; // ferma loop accettazione
        // chiude socket in ascolto
        if (close_server_fd >= 0) {
            close(close_server_fd);
            close_server_fd = -1;
        }
        // attende che processi figli terminano
        printf("Attesa terminazione processi figli\n");
        pid_t w;
        while (1) {
            w = waitpid(-1, NULL, 0);
            if (w > 0) continue;
            if (w == -1 && errno == EINTR) continue;
            break;
        }
        sleep(1); // tempo per figli di terminare
        printf("Terminazione completata\n");
        exit(EXIT_SUCCESS);
    } else if (sg == SIGALRM) {
        // SIGALRM: timer scaduto controllo dimensione log
        sigalrm_on = 1; // gestito in accept_client()
    }
}

// pulizia
void clean() {
    printf("Pulizia risorse\n");
    if (close_server_fd >= 0) {
        close(close_server_fd); // chiudi socket server
        close_server_fd = -1;
    }
    printf("Pulizia completata\n");
}



