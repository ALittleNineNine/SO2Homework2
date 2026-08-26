# include "aggregator.h"

int main() {
    // crea file log
    create_file_log();

    // crea server
    int server_fd = create_server();
    if (server_fd < 0) {
        fprintf(stderr, "Errore: creazione server fallita\n");
        return EXIT_FAILURE;
    }

    // signal handler
    set_signal(server_fd);

    // accetta client
    accept_client(server_fd);
    
    // puliza
    clean();
    return EXIT_SUCCESS;
}
