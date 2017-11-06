/*
Meno:
Datum:

Simulujte nasledujucu situaciu. Dvaja nastenkari menia raz za cas nastenku.
 Menenie nastenky trva nejaky cas (v simulacii 3s) a potom si idu pripravovat dalsiu zmenu nastenky (v simulacii 5s).
 Desiati zamestnanci si chodia pozerat nastenku, pricom pozeranie im trva nejaky cas (v simulacii 1s) a potom idu
 trosku popracovat (v simulacii 2s). Cela simulacia nech trva 30s.

1. Doplnte do programu premennu pocitajucu zmeny nastenky, po skonceni simulacie vypiste jej hodnotu. [2b]

2. Zabezpecte, aby iba jeden nastenkar mohol menit nastenku a to len vtedy, ked si ziaden zamestnanec nepozera nastenku.
 Zamestnanci si mozu naraz pozerat nastenku viaceri. [5b]

3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu tak, aby nastenkar ani zamestnanec po
 stanovenom case uz ziadnu cinnost nezacal. [3b]

Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi alebo s nahodne generovanymi casmi
- build (console): gcc nastenkari.c -o nastenkari -lpthread
*/

// TODO: reader & writer, reader preference

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std::chrono_literals;

const auto PREPARATION_TIME = 5s;
const auto CHANGING_TIME = 3s;

const auto READING_TIME = 1s;
const auto WORKING_TIME = 2s;

const auto TOTAL_TIME = 30s;

int totalChangeCounter = 0;

int employeeCount;
int activenoticeBoardManCount;

const int MAXIMUM_NASTENKAR = 1;

std::mutex changeCounterMutex;

std::mutex noticeBoardMutex;

std::mutex activenoticeBoardManMutex;

std::condition_variable noticeBoardMonitor;
std::condition_variable activenoticeBoardManMonitor;

// signal na zastavenie simulacie
bool stoj = false;

// nastenkar
void nastenkar_men() {
    std::this_thread::sleep_for(CHANGING_TIME);

    std::unique_lock<std::mutex> lock(changeCounterMutex);
    totalChangeCounter++;
    lock.unlock();
}

void nastenkar_pripravuj() {
    std::this_thread::sleep_for(PREPARATION_TIME);
}

void nastenkar() {

    while (!stoj) {

        std::unique_lock<std::mutex> nastenkaLock(noticeBoardMutex);
        noticeBoardMonitor.wait(nastenkaLock, [] { return employeeCount == 0; });


        std::unique_lock<std::mutex> activeNastenkarLock(activenoticeBoardManMutex);
        activenoticeBoardManMonitor.wait(activeNastenkarLock,
                                         [] { return activenoticeBoardManCount < MAXIMUM_NASTENKAR; });
        activenoticeBoardManCount++;
        activeNastenkarLock.unlock();

        if (!stoj) {
            nastenkar_men();
        }

        activeNastenkarLock.lock();
        activenoticeBoardManCount--;
        activenoticeBoardManMonitor.notify_all();
        activeNastenkarLock.unlock();

        nastenkaLock.unlock();

        if (stoj) {
            break;
        }

        nastenkar_pripravuj();
    }
}

// zamestnanec
void zamestnanec_citaj() {
    std::this_thread::sleep_for(READING_TIME);
}

void zamestnanec_pracuj() {
    std::this_thread::sleep_for(WORKING_TIME);
}

void zamestnanec() {

    while (!stoj) {
        std::unique_lock<std::mutex> nastenkaLock(noticeBoardMutex);
        employeeCount++;
        nastenkaLock.unlock();

        if (!stoj) {
            zamestnanec_citaj();
        }

        nastenkaLock.lock();
        employeeCount--;

        if (employeeCount == 0) {
            noticeBoardMonitor.notify_all();
        }
        nastenkaLock.unlock();

        if (stoj) {
            break;
        }

        zamestnanec_pracuj();
    }
}

// main f.
int main() {
    int i;

    std::thread nastenkari[2];
    std::thread zamestnanci[10];

    for (i = 0; i < 2; i++) {
        nastenkari[i] = std::thread(nastenkar);
    }

    for (i = 0; i < 10; i++) {
        zamestnanci[i] = std::thread(zamestnanec);
    }

    std::this_thread::sleep_for(TOTAL_TIME);

    std::cout << "Koniec simulacie" << std::endl;
    stoj = true;

    for (auto &item : nastenkari) {
        item.join();
    }

    for (auto &item : zamestnanci) {
        item.join();
    }

    std::cout << "Pocet zmenenych nastenok " << totalChangeCounter << std::endl;

    exit(EXIT_SUCCESS);
}