# Esempi di test

## Compilazione:
```bash
gcc -Wall main.c aggregator.c -o aggregator
gcc -Wall client.c -o client
```

## Test avvio aggregator:
Eseguire `aggregator` in un terminale:
```bash
./aggregator
```
- Il terminale mostra `Server in ascolto sulla porta 6666`;
- `aggregator` è in continuo running in attesa del client.

**Test passed**.

## Test connessione client:
Dopo aver eseguito `aggregator` in un terminale (come sopra).

### Un client:
- Eseguire `client` in un altro terminale:
```bash
./client localhost
```
- Il terminale aggregator mostra:
```text
Client 1 connesso
```
- Il terminale client mostra:
```text
Connessione al aggregatore con successo
```

**Test passed**.

### Più client:
- Eseguire `aggregator` in un terminale:
```bash
./aggregator
```
- Eseguire 3 `client` in altri 3 terminali:
```bash
./client localhost
```
```bash
./client localhost
```
```bash
./client localhost
```
- Il terminale aggregator mostra:
```text
Client 1 connesso
Client 2 connesso
Client 3 connesso
```

**Test passed**.

## Test invio di dati validi:
Dopo aver eseguito `aggregator` in un terminale e `client` in un altro terminale (come sopra).

### Un dato:
- Digitando nel terminale client:
```text
1 111
```
- Il `client` invia `[1, 111]` al `aggregator`;
- Il terminale aggregator mostra:
```text
Client 1: [1, 111]
```
- L'`aggregator` scrive in `logs/logfile.txt`:
```text
[TIMESTAMP, 1, 111]
```
- Può essere approvato con:
```bash
cat logs/logfile.txt
```

**Test passed**.

### Più dati:
- Digitando nel terminale client:
```text
1 111
2 222
3 333
```
- Il `client` invia di volta in volta `[1, 111]`, `[2, 222]`, `[3, 333]` al `aggregator`;
- Il terminale aggregator mostra:
```text
Client 2: [1, 111]
Client 2: [2, 222]
Client 2: [3, 333]
```
- L'`aggregator` scrive in `logs/logfile.txt`:
```text
[TIMESTAMP, 1, 111]
[TIMESTAMP, 2, 222]
[TIMESTAMP, 3, 333]
```
- Può essere approvato con:
```bash
cat logs/logfile.txt
```

**Test passed**.

## Test disconnessione client:
Dopo aver eseguito `aggregator` in un terminale, `client` in un altro terminale e scritto qualcosa (come sopra):

- Digitando `Ctrl+D` termina il `client`;
- Il terminale aggregator mostra:
```text
Client 1 disconnesso
```
- L'`aggregator` scrive in `logs/logfile.txt`:
```text
[TIMESTAMP, 1, DISCONNECT]
```
- Può essere approvato con:
```bash
cat logs/logfile.txt
```

**Test passed**.

## Test scritture concorrenti:

Dopo aver eseguito `aggregator` in un terminale (come sopra).

Compilare `client_send_BOOM.c` in un altro terminale:
```bash
gcc -Wall client_send_BOOM.c -o client_send_BOOM
```
Eseguire contemporaneamente 3 `client_send_BOOM` in background:
```bash
./client_send_BOOM localhost & ./client_send_BOOM localhost & ./client_send_BOOM localhost &
```
Il terminale client mostra i numeri di job e i PID dei 3 `client`:
```text
[JOB_NUMBER] PID
[JOB_NUMBER] PID
[JOB_NUMBER] PID
```
Il terminale aggregator mostra che i 3 `client` si connettono e continuano a stampare `Client n: [9, 99999]` in ordine sparso (perché in multi-process):
```text
Client 1 connesso
Client 2 connesso
Client 3 connesso
Client 1: [9, 99999]
Client 2: [9, 99999]
Client 3: [9, 99999]
Client 1: [9, 99999]
Client 3: [9, 99999]
Client 2: [9, 99999]
...
```
- L'`aggregator` scrive continuamente in `logs/logfile.txt` senza problemi di concorrenza:
```text
[TIMESTAMP, 9, 99999]
...
```
- Può essere approvato con:
```bash
cat logs/logfile.txt
```

**Test passed**.

## Test dimensione massima e rotazione logs:

Dopo aver eseguito `aggregator` in un terminale, tanti `client_send_BOOM` contemporaneamente in background in un altro terminale **per un po' di tempo** (come sopra).

Si può accorgere che nel file `logs` contiene oltre al file `logfile.txt`, sono comparsi altri `logfile.txt_TIMESTAMP.log`.

- Può essere approvato con:
```bash
ls logs
```

**Test passed**.

## Test messaggio non valido:

Dopo aver eseguito `aggregator` in un terminale (come sopra).

Eseguire in un altro terminale:
```bash
telnet localhost 6666
```
- Poi digitare dei messaggi non validi, ad esempio:
```text
hello world
```
- Il terminale aggregator mostra:
```text
Client 1: hello world
Messaggio non valido: hello world
```

**Test passed**.

## Test SIGPIPE:

Il nostro aggregator non usa `write` sul socket, quindi questo non è verificabile.

Comunque nel codice viene mostrato come verrà gestito quando viene segnalato `SIGPIPE`.

## Test SIGINT:

Dopo aver eseguito `aggregator` in un terminale (come sopra).

### Senza client:

- Digitare `Ctrl+C` (inviando `SIGINT`);
- Il terminale aggregator mostra:
```text
Segnale SIGINT - Terminazione controllata
Attesa terminazione processi figli
Terminazione completata
```
- L'`aggregator` termina, verificabile con:
```bash
ps
```

**Test passed**.

### Con client:

- Eseguire `client` in un altro terminale:
```bash
./client localhost
```
- Inserire un po' di dati, ad esempio:
```text
1 111
2 222
3 333
```
- Digitare `Ctrl+C` nel terminale aggregator (inviando `SIGINT`);
- Il terminale aggregator mostra:
```text
Segnale SIGINT - Terminazione controllata
Attesa terminazione processi figli
```
- Solo quando tutti i processi figli terminano, verrà stampata:
```text
Terminazione completata
```
- L'`aggregator` e tutti i suoi figli sono terminati, verificabile con:
```bash
ps
```

**Test passed**.

## Test riutilizzo rapido IP:PORT:

Eseguire `aggregator` in un terminale.

Terminarlo con `Ctrl+C` (`SIGINT`).

Rieseguire velocemente `aggregator` nello stesso terminale.

L'esecuzione avviene correttamente.

**Test passed**.



