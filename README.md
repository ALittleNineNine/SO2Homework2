# SO2Homework2
Il sistema si basa su un'architettura Client/Server multi-processo o multi-thread per la gestione del log di produzione.
Ruolo:
    - produttori (client): inviano messaggi contenenti dati numerici e un identificativo univoco;
    - coordinatore (aggregatore): server TCP in ascolto su una specifica porta; accetta le connessione dai client e gestisce dati.
    
Il coordinatore deve usare processi o thread multipli per gestire le connessioni client in ingresso.

## 1. Architettura Principale
Creare dei file:
- *aggregator.h* (header) che contiene:
    - le **librerie standard** utilizzate nel codice;
    - le definizioni delle strutture **struct**;
    - i **prototipi** delle funzioni che verranno implementate in un file.c;
- *aggregator.c* (funzioni) che contiene:
    - logica di gestione del file di log (scrittura, calcolo timestamp, rotazione);
    - handler dei segnali SIGALRM, SIGINT, SIGPIPE;
- *main.c* che contiene il **main**:
    - gestione degli argomenti da riga di comando;
    - inizializzazione del socket TCP;
    - ciclo di accettazione delle connessioni con la generazione dei figli.
- eventuale *client.c* che contiene i test.

## 2. Strutture Dati
Creare delle struct per gestire:
- configurazione del server (memorizza i parametri del server):
    - porta di ascolto;
    - dimensione massima del file di log;
    - percorso del file di log corrente.
- gestione connessione:
    - struttura per i thread/processi figli;
    - informazioni sui clienti connessi.
    
## 3. Argomenti Riga Comandi
Il server riceverà parametri come:
    - p, -- port: porta di ascolto del server;
    - s, -- size: dimensione massima del file di log in bytes;
    - l, -- logdir: directory dove salvare i file di log.
    
## 4. Gestione del Server TCP
Il coordinatore deve:
    - creare il socket;
    - impostare il riuso dell'indirizzo;
    - bindare l'indirizzo e la porta;
    - mettersi in ascolto.

## 5. Gestione dei Thread/Processi
Il server può usare thread o processi e gestisce le richieste in ingresso.

## 6. Gestione dei Segnali
Il server deve gestire i seguenti segnali:
    - SIGPIPE: non deve far crashare il server se un client si disconnette all'improvviso durante una scrittura, è necessario intercettarlo e scrivere nel log la  stringa richiesta: [TIMESTAMP, ID_MITTENTE, "DISCONNECT"];
    - SIGALRM: scatta ogni X secondi, controlla la dimensione del file e se supera la soglia avvia la rotazione;
    - SIGINT: deve smettere di accettare nuove connessioni e attendere che i client connessi finiscano di scrivere e chiudere in sicurezza il file di log.
    
## 7. Gestione dei File di Log
Il coordinatore deve:
    - scrivere in modalità append;
    - utilizzare lock di scrittura;
    - controllare la dimensione (SIGALRM);
    - rotazione dei log.

## 8. Comunicazione Client-Server
Formato del messaggio del client: [ID_MITTENTE, DATO]
Il server riceve e legge il messaggio, estae i campi, calcola l'ora corrente del sistema e formatta [TIMESTAMP, ID_MITTENTE, DATO] prima di effetturare il lock e la scrittura.

## 9. Test
Creare test per verifica il server.
Ad esempio:
    - test connessione singola;
    - test connessioni multiple;
    - test gestione SIGPIPE;
    - test rotazione dei log;
    - test riuso di IP:PORT;
    - test gestione SIGINT controllata ecc..
    
