/*
Meno:
Datum:

Simulujte nasledujcu situaciu. Dvadsat sportovcov (desat muzov a desat zien) spolu trenuju beh. Kazdy sportovec po
 kazdom odbehnutom okruhu si oddychne a potom zase ide na start a zase bezi. Muzovi beh trva nejaky cas (v simulacii 2s)
 a potom oddychuje (v simulacii 2s). Zene beh trva nejaky cas (v simulacii 3s) a potom oddychuje (v simulacii 3s).

1. Doplnte do programu premennu pocitajucu pocet dokoncenych okruhov, ktory spolu vsetci sportovci odbehli (jedno
 pocitadlo pre celu skupinu). [2b]

2. Zabezpecte ukoncenie simulacie, ked kazdy muz odbehne 15 okruhov a kazda zena odbehne 10 okruhov. [2b]

3. Zabezpecte, aby sa vsetci sportovci (co budu bezat dalsi okruh) po oddychu na starte pockali a az potom vystartovali
 do dalsieho kola. [6b]

Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi alebo s nahodne generovanymi casmi
- build (console): gcc sportovci.c -o sportovci -lpthread
*/

#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>

using namespace std::chrono_literals;

const auto MAN_RUNNING_TIME = 2s;
const auto MAN_BREAK_TIME = 2s;
const auto WOMAN_RUNNING_TIME = 3s;
const auto WOMAN_BREAK_TIME = 3s;

int runningCount = 20;

int totalLapCounter = 0;

std::mutex paintMutex;
std::mutex breakMutex;

std::mutex runningMutex;
std::mutex lapCounterMutex;
std::condition_variable runningBarrierCond;

int waitingCount = 0;

// signal na zastavenie simulacie
bool stoj = false;

bool inBarrier = false;

void muz_bez(void) {
    std::this_thread::sleep_for(MAN_RUNNING_TIME);
}

//  muz
void muz_oddychuj(void) {
    std::this_thread::sleep_for(MAN_BREAK_TIME);
}

void muz(int id) {

    int lapCount = 0;
    const int maximumLapCount = 15;

    while (!stoj) {

        if (lapCount == maximumLapCount) {
            std::cout << "Muz " << id << " skoncil." << std::endl;
            break;
        }

        std::unique_lock<std::mutex> barrierLock(runningMutex);

        std::cout << "Muz " << id << " je pripraveny " << std::endl;

        // in
        waitingCount++;
        if (waitingCount != runningCount) {
            runningBarrierCond.wait(barrierLock, [] { return inBarrier; });
        } else {
            inBarrier = true;
            runningBarrierCond.notify_all();
        }

        // bude toto moje posledne kolo?
        if (lapCount + 1 == maximumLapCount) {
            runningCount--;
        }

        // out
        waitingCount--;
        if (waitingCount != 0) {
            runningBarrierCond.wait(barrierLock, [] { return !inBarrier; });
        } else {
            inBarrier = false;
            runningBarrierCond.notify_all();
        }

        //
        barrierLock.unlock();

        muz_bez();

        lapCount++;

        std::unique_lock<std::mutex> lapCounterLock(lapCounterMutex);
        std::cout << "Muz " << id << " uberhol " << lapCount << ". kolo" << std::endl;
        totalLapCounter++;
        lapCounterLock.unlock();


        muz_oddychuj();
    }
}

// zena
void zena_bez() {
    std::this_thread::sleep_for(WOMAN_RUNNING_TIME);
}

void zena_oddychuj() {
    std::this_thread::sleep_for(WOMAN_BREAK_TIME);
}

void zena(int id) {

    int lapCount = 0;
    const int maximumLapCount = 10;

    while (!stoj) {
        if (lapCount == maximumLapCount) {
            std::cout << "Zena " << id << " skoncila." << std::endl;
            break;
        }

        std::unique_lock<std::mutex> barrierLock(runningMutex);

        std::cout << "Zena " << id << " je pripravena " << std::endl;

        // in
        waitingCount++;
        if (waitingCount != runningCount) {
            runningBarrierCond.wait(barrierLock, [] { return inBarrier; });
        } else {
            inBarrier = true;
            runningBarrierCond.notify_all();
        }

        // bude toto moje posledne kolo?
        if (lapCount + 1 == maximumLapCount) {
            runningCount--;
        }

        // out
        waitingCount--;
        if (waitingCount != 0) {
            runningBarrierCond.wait(barrierLock, [] { return !inBarrier; });
        } else {
            inBarrier = false;
            runningBarrierCond.notify_all();
        }

        //
        barrierLock.unlock();

        zena_bez();

        lapCount++;

        std::unique_lock<std::mutex> lapCounterLock(lapCounterMutex);
        std::cout << "Zena " << id << " uberhla " << lapCount << ". kolo" << std::endl;
        totalLapCounter++;
        lapCounterLock.unlock();


        zena_oddychuj();
    }
}

// main f.
int main() {
    int i;

    std::thread zeny[10];
    std::thread muzi[10];

    for (i = 0; i < 10; i++) {
        zeny[i] = std::thread(zena, i);
        muzi[i] = std::thread(muz, i);
    }

    for (i = 0; i < 10; i++) {
        zeny[i].join();
        muzi[i].join();
    }

    std::cout << "Total laps: " << totalLapCounter << std::endl;


    exit(EXIT_SUCCESS);
}