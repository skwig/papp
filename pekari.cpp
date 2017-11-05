/*
Meno:
Datum:

Simulujte nasledujucu situaciu. V pekarni pracuju pekari (10 pekarov), ktori pecu chlieb v peciach (4 pece).
 Pekar pripravuje chlieb nejaky cas (v simulacii 4s) a potom ide k volnej peci a pecie v nej chlieb (2s).
 Cela simulacia nech trva 30s.

1. Doplnte do programu pocitadlo pocitajuce, kolko chlebov bolo upecenych. [2b]

2. Zabezpecte, aby do obsadenej pece pekar vlozil chlieb az ked sa uvolni, cize aby pockal, kym nebude nejaka pec volna.
Simulujte situaciu, ze ked pekar upecie 2 chleby, pocka na vsetkych kolegov a spravia si prestavku (v simulacii 4s). [5b]

3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu tak, aby pekar prerusil cinnost hned,
ako je to mozne (ak uz zacal pripravu alebo pecenie moze ju dokoncit).  [3b]

Poznamky:
- na synchronizaciu pouzite iba mutexy, podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc pekari.c -o pekari -lpthread
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std::chrono_literals;

// signal na zastavenie simulacie
bool run = false;

const auto PREPARATION_TIME = 4s;
const auto BREAK_TIME = 4s;
const auto BAKING_TIME = 2s;
const auto TOTAL_TIME = 30s;

const int BAKER_COUNT = 10;
const int OVEN_COUNT = 4;
const int BREAD_BREAK_COUNT = 2;

int breadCount;
int breakCount;
int emptyOvenCount;

bool inBarrier = false;

std::mutex breadMutex;
std::mutex breakMutex;
std::mutex ovenMutex;

std::condition_variable ovenMonitor;
std::condition_variable breakBarrier;

// pekar
void priprava() {
    std::this_thread::sleep_for(PREPARATION_TIME);
}

void pecenie() {

    std::unique_lock<std::mutex> ovenLock(ovenMutex);
    while (emptyOvenCount <= 0) {
        ovenMonitor.wait(ovenLock);
    }
    emptyOvenCount--;
    ovenLock.unlock();

    std::this_thread::sleep_for(BAKING_TIME);

    ovenLock.lock();
    breadCount++;
    emptyOvenCount++;
    ovenLock.unlock();

    ovenMonitor.notify_all();
}

void pekar() {
    int bakerBreadCount = 0;

    while (run) {
        priprava();

        if (!run) {
            break;
        }

        pecenie();

        bakerBreadCount++;

        if (bakerBreadCount % BREAD_BREAK_COUNT == 0) {
            std::unique_lock<std::mutex> breakLock(breakMutex);

            // in
            breakCount++;
            if (breakCount != BAKER_COUNT) {
                while (!inBarrier) { breakBarrier.wait(breakLock); }
            } else {
                inBarrier = true;
                breakBarrier.notify_all();
            }

            // out
            breakCount--;
            if (breakCount != 0) {
                while (inBarrier) { breakBarrier.wait(breakLock); }
            } else {
                inBarrier = false;
                breakBarrier.notify_all();
            }

            breakLock.unlock();

            //
            std::this_thread::sleep_for(BREAK_TIME);
        }

    }
}

int main() {


    //
    emptyOvenCount = OVEN_COUNT;
    breadCount = 0;
    breakCount = 0;

    //
    std::thread pekari[BAKER_COUNT];

    for (int i = 0; i < BAKER_COUNT; ++i) {
        pekari[i] = std::thread(pekar);
    }

    std::this_thread::sleep_for(TOTAL_TIME);
    run = false;

    for (auto &pekar : pekari) {
        pekar.join();
    }

    std::cout << "Total bread count " << breadCount << std::endl;

//    int i;
//
//    pthread_t pekari[10];
//
//    for (i=0;i<10;i++) pthread_create( &pekari[i], NULL, &pekar, NULL);
//
//    sleep(30);
//    stoj = 1;
//
//    for (i=0;i<10;i++) pthread_join( pekari[i], NULL);


    exit(EXIT_SUCCESS);
}