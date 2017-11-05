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

#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>

using namespace std::chrono_literals;

const auto WORK_TIME = 2s;
const auto PAINT_TIME = 1s;
const auto TOTAL_TIME = 30s;
const auto BREAK_TIME = 2s;

const int PAINTER_COUNT = 10;
const int PAINTER_BREAK_COUNT = 3;
const int PAINT_CAN_BREAK_AMOUNT = 4;

int totalPaintCount;

int breakCount;

std::mutex paintMutex;
std::mutex breakMutex;

std::condition_variable breakMonitor;
std::condition_variable breakBarrier;

bool havingABreak = false;
bool inBarrier = false;
bool stop = false;


// maliar - malovanie steny
void maluj() {
    std::this_thread::sleep_for(WORK_TIME);
}

//  maliar - branie farby
void ber() {

    std::unique_lock<std::mutex> paintLock(paintMutex);
    totalPaintCount++;
    paintMutex.unlock();

    std::this_thread::sleep_for(PAINT_TIME);
}

// maliar
void maliar() {

    int thisPaintCount = 0;

    // pokial nie je zastaveny
    while (!stop) {
        maluj();

        if (stop) {
            break;
        }

        ber();

        thisPaintCount++;

        if (stop) {
            break;
        }

        if (thisPaintCount % PAINT_CAN_BREAK_AMOUNT == 0) {
            std::unique_lock<std::mutex> breakLock(breakMutex);
//            while (havingABreak) {
//                breakMonitor.wait(breakLock);
//            }

            // in
            breakCount++;
            if (breakCount != PAINTER_BREAK_COUNT) {
                while (!inBarrier) { breakBarrier.wait(breakLock); }
            } else {
                havingABreak = true;
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


            //
            breakLock.unlock();

            std::this_thread::sleep_for(BREAK_TIME);

            havingABreak = false;

            std::cout << breakCount << std::endl;

//            breakMonitor.notify_all();
        }


    }

    std::cout << "Maliar minul " << thisPaintCount << std::endl;
}

int main() {
    int i;


    totalPaintCount = 0;

    std::thread maliari[10];

    for (i = 0; i < 10; i++) {
        maliari[i] = std::thread(maliar);
    }

    std::this_thread::sleep_for(TOTAL_TIME);
    stop = true;

    std::cout << "Koniec simulacie" << std::endl;

    for (auto &maliar : maliari) {
        maliar.join();
    }

    std::cout << "Spolu minuli " << totalPaintCount << std::endl;

    exit(EXIT_SUCCESS);
}