/*
Meno:
Datum:

Simulujte nasledujucu situaciu. V kmeni su dve kasty: lovci (6 lovcov) a zberaci (12 zberacov). Uctievaju bozstvo,
 ktoremu chodia davat dary do chramu. Lovec lovi zver nejaky cas (v simulacii 6s) a potom ide do chramu dat cast ulovku
 ako dar bozstvu, co tiez trva nejaky cas (v simulacii 2s). Zberac zbiera plody nejaky cas (v simulacii 4s) a potom ide
 do chramu dat cast plodov bozstvu, co tiez trva nejaky cas (v simulacii 1s). Cela simulacia nech trva 30s.

1. Doplnte do programu pocitadlo pocitajuce, kolko krat bozstvu dali dar lovci a kolko krat zberaci. [2b]

2. Zabezpecte, aby do chramu sucasne mohli vojst maximalne dvaja lovci alebo styria zberaci, iba prislusnici jednej
 kasty naraz. Ak je pred chramom rad, zabezpecte spravodlivy pristup (kasty su si rovnocenne). [5b]

3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu. [3b]

Poznamky:
- na synchronizaciu pouzite iba mutexy, podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc lovci_a_zberaci.c -o lovci_a_zberaci -lpthread
*/

// TODO reader & writer, rovnomerne

#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>

using namespace std::chrono_literals;

const auto HUNTING_TIME = 6s;
const auto OFFER_HUNT_TIME = 2s;
const auto GATHERING_TIME = 4s;
const auto OFFER_GATHER_TIME = 1s;
const auto TOTAL_TIME = 30s;

const int MAXIMUM_HUNTER_COUNT = 2;
const int MAXIMUM_GATHERER_COUNT = 4;

int totalHunterCounter = 0;
int totalGathererCounter = 0;

int hunterCount = 0;
int gathererCount = 0;
int activeHunterCount = 0;
int activeGathererCount = 0;

std::mutex gathererCounterMutex;
std::mutex hunterCounterMutex;

std::mutex offeringMutex;
std::mutex activeHuntersMutex;
std::mutex activeGatherersMutex;

std::condition_variable noHuntersMonitor;
std::condition_variable noGatherersMonitor;
std::condition_variable activeHuntersMonitor;
std::condition_variable activeGatherersMonitor;

bool inBarrier = false;
bool stoj = false;


// lovec
void lov() {
    std::this_thread::sleep_for(HUNTING_TIME);
}

void dar_lov() {
    std::this_thread::sleep_for(OFFER_HUNT_TIME);

    std::unique_lock<std::mutex> lock(hunterCounterMutex);
    totalHunterCounter++;
    lock.unlock();
}

void lovec() {

    while (!stoj) {
        lov();

        if (stoj) {
            break;
        }

        //
        std::unique_lock<std::mutex> offeringLock(offeringMutex);
        noGatherersMonitor.wait(offeringLock, [] { return gathererCount == 0; });

        if(stoj){
            break;
        }

        hunterCount++;
        offeringLock.unlock();

        //
        std::unique_lock<std::mutex> activeLock(activeHuntersMutex);
        activeHuntersMonitor.wait(activeLock, [] { return activeHunterCount < MAXIMUM_HUNTER_COUNT; });
        activeHunterCount++;
        activeLock.unlock();

        if (!stoj) {
            dar_lov();
        }

        activeLock.lock();
        activeHunterCount--;
        activeHuntersMonitor.notify_all();
        activeLock.unlock();

        //
        offeringLock.lock();
        hunterCount--;
        if (hunterCount == 0) {
            noHuntersMonitor.notify_all();
        }
        offeringLock.unlock();
    }
}

// zberac
void zber() {
    std::this_thread::sleep_for(GATHERING_TIME);
}

void dar_zber() {
    std::this_thread::sleep_for(OFFER_GATHER_TIME);

    std::unique_lock<std::mutex> lock(gathererCounterMutex);
    totalGathererCounter++;
    lock.unlock();
}

void zberac() {

    // pokial nie je zastaveny
    while (!stoj) {
        zber();

        if (stoj) {
            break;
        }

        //
        std::unique_lock<std::mutex> offeringLock(offeringMutex);
        noHuntersMonitor.wait(offeringLock, [] { return hunterCount == 0; });

        if(stoj){
            break;
        }

        gathererCount++;
        offeringLock.unlock();


        //
        std::unique_lock<std::mutex> activeLock(activeGatherersMutex);
        activeGatherersMonitor.wait(activeLock, [] { return activeGathererCount < MAXIMUM_GATHERER_COUNT; });
        activeGathererCount++;
        activeLock.unlock();


        if (!stoj) {
            dar_zber();
        }

        activeLock.lock();
        activeGathererCount--;
        activeGatherersMonitor.notify_all();
        activeLock.unlock();

        //
        offeringLock.lock();
        gathererCount--;
        if (gathererCount == 0) {
            noGatherersMonitor.notify_all();
        }
        offeringLock.unlock();
    }
}

int main() {
    int i;

    std::thread lovci[6];
    std::thread zberaci[12];

    for (i = 0; i < 6; i++) {
        lovci[i] = std::thread(lovec);
    }
    for (i = 0; i < 12; i++) {
        zberaci[i] = std::thread(zberac);
    }

    std::this_thread::sleep_for(TOTAL_TIME);
    std::cout << "Koniec simulacie." << std::endl;
    stoj = true;

    for (auto &item : lovci) {
        item.join();
    }

    for (auto &item : zberaci) {
        item.join();
    }

    std::cout << "Lovci: " << totalHunterCounter << std::endl;
    std::cout << "Zberaci: " << totalGathererCounter << std::endl;

    exit(EXIT_SUCCESS);
}