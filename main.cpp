/*
Meno:
Datum:

Simulujte nasledujucu situaciu. V pekarni pracuju pekari (10 pekarov), ktori pecu chlieb v peciach (4 pece). Pekar pripravuje chlieb nejaky cas (v simulacii 4s) a potom ide k volnej peci a pecie v nej chlieb (2s). Cela simulacia nech trva 30s.

1. Doplnte do programu pocitadlo pocitajuce, kolko chlebov bolo upecenych. [2b]

2. Zabezpecte, aby do obsadenej pece pekar vlozil chlieb az ked sa uvolni, cize aby poclal, kym nebude nejaka pec volna. Simulujte situaciu, ze ked pekar upecie 2 chleby, pocka na vsetkych kolegov a spravia si prestavku (v simulacii 4s). [5b]

3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu tak, aby pekar prerusil cinnost hned, ako je to mozne (ak uz zacal pripravu alebo pecenie moze ju dokoncit).  [3b]

Poznamky:
- na synchronizaciu pouzite iba mutexy, podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc pekari.c -o pekari -lpthread
*/

#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <mutex>

using namespace std::chrono_literals;

const int FORGE_COUNT = 4;
int emptyForge = 4;
std::condition_variable forgeCond;
std::mutex condMutex;

std::condition_variable pauseCond;
std::mutex breakMutex;
int breakCount = 0;

std::mutex stopMutex;
bool stop = false;

int totalBreads = 0;


void priprava() {
    std::this_thread::sleep_for(std::chrono::seconds(4));
}

void pecenie() {
    std::unique_lock<std::mutex> lock(condMutex);
    forgeCond.wait(lock, [] { return emptyForge > 0; });
    --emptyForge;
    lock.unlock();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    lock.lock();
    ++emptyForge;
    ++totalBreads;
    forgeCond.notify_one();


    //std::unique_lock<std::mutex> lock(mutex);
    //++emptyForge;
    //++totalBreads;
    //forgeCond.notify_one();
}

void pekar() {
    int breadCount = 0;

    while (!stop) {
        priprava();

        std::unique_lock<std::mutex> lock(stopMutex);
        if (stop)
            break;
        lock.unlock();

        pecenie();
        breadCount++;

        if (breadCount % 2 == 0) {
            std::unique_lock<std::mutex> lock(breakMutex);
            ++breakCount;
            std::cout << "Pause: " << breakCount << std::endl;
            if (breakCount % 10 == 0) {
                pauseCond.notify_all();
            } else {
                pauseCond.wait(lock, [] { return breakCount % 10 == 0; });
            }
            lock.unlock();
            std::cout << "All came" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
        }
    }
}

int main() {
    std::thread pekari[10];

    for (int i = 0; i < 10; ++i)
        pekari[i] = std::thread(pekar);

    std::this_thread::sleep_for(std::chrono::seconds(30));
    stop = true;

    for (int i = 0; i < 10; ++i)
        pekari[i].join();

    std::cout << "Celkovo chlebov: " << totalBreads << std::endl;

    getchar();
    return EXIT_SUCCESS;
}