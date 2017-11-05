/*
Meno:
Datum:

Simulujte nasledujucu situaciu. Desiati maliari maluju steny.
Maliarovi trva nejaky cas, kym stenu maluje (v simulacii 2s) a nejaky cas, kym si ide nabrat farbu do vedra (v simulacii 1s).
Cela simulacia nech trva nejaky cas (30s).

1. Doplnte do programu pocitadlo celkoveho poctu vedier minutej farby a tiez nech si kazdy maliar pocita,
kolko vedier farby uz minul preniesol, na konci simulacie vypiste hodnoty pocitadiel. [2b]

2. Ked maliar minie 4 vedra, pocka na dvoch svojich kolegov a kazdy si spravi prestavku na nejaky cas (v simulacii 2s). [5b]

3. Osetrite v programe spravne ukoncenie simulacie hned po uplynuti stanoveneho casu (nezacne sa dalsia cinnost). [3b]


Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc maliari.c -o maliari -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


// signal na zastavenie
int stoj = 0;

// maliar - malovanie steny
void maluj(void) {
    sleep(2);
}

//  maliar - branie farby
void ber(void) {
    sleep(1);
}

// maliar
void *maliar( void *ptr ) {

    // pokial nie je zastaveny
    while(!stoj) {
        maluj();
        ber();
    }
    return NULL;
}

int main(void) {
    int i;

    pthread_t maliari[10];

    for (i=0;i<10;i++) pthread_create(&maliari[i], NULL, &maliar, NULL);

    sleep(30);
    stoj = 1;

    for (i=0;i<10;i++) pthread_join(maliari[i], NULL);

    exit(EXIT_SUCCESS);
}