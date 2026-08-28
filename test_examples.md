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

## Test scritture concorrenti:

[da scrivere]

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

## Test messaggio non valido:

[da scrivere]

## Test dimensione massima e rotazione logs:

[da scrivere]

## Test SIGINT senza client:

[da scrivere]

## Test SIGINT con client:

[da scrivere]

## Test riutilizzo rapido IP:PORT:

[da scrivere]



