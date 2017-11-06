/*
Meno:
Datum:

Simulujte nasledujucu situaciu. V malom kralovstve korunovali noveho krala a chodia sa mu neustale klanat styria
slachtici a desiati poddani. Prejavovanie ucty kralovi trva nejaky cas (v simulacii 1s) a nejaky cas si slahctic
ci poddany dava prestavku (v simulacii 4s). Cela simulacia nech trva 30s.

1. Doplnte do programu pocitadlo pocitajuce, kolko krat sa kralovi poklonili slachtici; a pocitadlo pocitajuce,
kolko krat sa kralovi poklonili poddani. [2b]

2. Zabezpecte, aby sa kralovi sucasne klanali maximalne dvaja slachtici a tiez aby sa kralovi neklanal slachtic
spolu s poddanym (cize alebo max. 2 slachtici, alebo lubovolne vela poddanych). Ak je pred kralom rad, slachtici maju
samozrejme prednost. [5b]

3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu. [3b]

Poznamky:
- na synchronizaciu pouzite iba mutexy, podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc poslovia_a_pisari -o poslovia_a_pisari -lpthread
*/

#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>

// TODO: reader & writer, writer preference

using namespace std::chrono_literals;
// signal na zastavenie simulacie
bool stop = false;

const auto BOWING_TIME = 1s;
const auto BREAK_TIME = 4s;
const auto TOTAL_TIME = 30s;

const int BAKER_COUNT = 10;
const int OVEN_COUNT = 4;
const int BREAD_BREAK_COUNT = 2;


int slaveCount = 0;
int lordCount = 0;
int activeLords = 0;

int slaveCounter = 0;
int lordCounter = 0;

const int MAXIMUM_LORD_BOWING_COUNT = 2;

std::mutex mutex;
std::mutex activeLordsMutex;

std::mutex lordCounterMutex;
std::mutex serfCounterMutex;

std::condition_variable slaveCond;
std::condition_variable lordCond;
std::condition_variable activeLordCond;

// klananie sa
void klananie() {
    std::this_thread::sleep_for(BOWING_TIME);
}

// prestavka medzi klananiami
void prestavka() {
    std::this_thread::sleep_for(BREAK_TIME);
}

// slachtic
void slachtic() {

    // pokial nie je zastaveny
    while (!stop) {

        std::unique_lock<std::mutex> lock(mutex);
        ++lordCount;
        while (!(slaveCount == 0)) {
            slaveCond.wait(lock);
        }
//        slaveCond.wait(lock, [] { return slaveCount == 0; });
        lock.unlock();

        std::unique_lock<std::mutex> activeLock(activeLordsMutex);
        while (!(activeLords < 2)) {
            activeLordCond.wait(activeLock);
        }
//        activeLordCond.wait(activeLock, [] { return activeLords < 2; });
        ++activeLords;
        activeLock.unlock();

        if (!stop) {
            klananie();
            std::unique_lock<std::mutex> lordCounterLock(lordCounterMutex);
            lordCounter++;
            lordCounterLock.unlock();
        }

        //
        activeLock.lock();
        --activeLords;
        activeLordCond.notify_one();
        activeLock.unlock();

//        bowingLordMonitor.notify_all();

        lock.lock();
        --lordCount;
        lordCond.notify_all();
        lock.unlock();

        if (stop) {
            break;
        }

        prestavka();
    }
}

// poddany
void poddany() {

    // pokial nie je zastaveny
    while (!stop) {

        std::unique_lock<std::mutex> lock(mutex);

//        if (!(lordCount == 0)) {
//            lordCond.wait(lock);
//        }
        lordCond.wait(lock, [] { return lordCount == 0; });
        ++slaveCount;
        lock.unlock();

        if (!stop) {
            klananie();
            std::unique_lock<std::mutex> slaveCounterLock(serfCounterMutex);
            slaveCounter++;
            slaveCounterLock.unlock();
        }

        lock.lock();
        --slaveCount;
        if (lordCount == 0) {
            slaveCond.notify_all();
        }
        lock.unlock();

        if (stop) {
            break;
        }

        prestavka();
    }
}

int main() {

    int i;

    std::thread slachtici[4];
    std::thread poddani[10];

    for (i = 0; i < 4; i++)
        slachtici[i] = std::thread(slachtic);

    for (i = 0; i < 10; i++)
        poddani[i] = std::thread(poddany);

    std::this_thread::sleep_for(30s);
    std::cout << "Koniec simulacie" << std::endl;
    stop = true;

    for (i = 0; i < 4; i++) {
        std::cout << "joining slachtic " << i << std::endl;
        slachtici[i].join();
    }

    for (i = 0; i < 10; i++) {
        std::cout << "joining poddany " << i << std::endl;
        poddani[i].join();
    }

    std::cout << "Slachtici sa poklonili " << lordCounter << " krat\n";
    std::cout << "Poddani sa poklonili " << slaveCounter << " krat\n";

    exit(EXIT_SUCCESS);
}