/*
https://goo.gl/ESYaxB

Meno:
Datum:

Simulujte nasledujucu situaciu. V obci su volby a 100 volicov ide volit do volebnej miestnosti, kde su 3 plenty a 1 volebna urna.
Volic prichadza do miestnosti (v simulacii kazdu 1s) najskor kruzkuje kandidatov (v simulacii 2s) a
nasledne vhadzuje hlasovaci listok do urny (v simulacii 1s). Simulacia konci ked odhlasuju vsetci volici.

1. Doplnte do programu premennu pocitajucu pocet volicov, ktori uz odvolili; hodnota nech je programom vypisovana kazdych 5s. [4b]

2. Zabezpecte synchronizaciu, tak, aby subezne mohli iba traja volici kruzkovat kandidatov za plentami a iba jeden volic vhadzovat listok do urny. [6b]

Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne; resp monitory
- nespoliehajte sa na uvedene casy ci pocty, simulacia by mala fungovat aj s inymi casmi alebo s nahodne generovanymi casmi alebo poctami
- build (console): gcc volici.c -o volici -lpthread
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std::chrono_literals;

const int POCET_VOLICOV = 100;

const auto PLETNA_TIME = 2s;
const auto URNA_TIME = 1s;
const auto VOTER_TIME = 1s;
const auto PRINT_PERIOD = 5s;

const int PLETNA_COUNT = 3;
const int URNA_COUNT = 1;

int emptyPletnaCount;
int emptyUrnaCount;
int voteCount;

bool print = true;

std::mutex urnaMutex;
std::mutex pletnaMutex;

std::condition_variable urnaMonitor;
std::condition_variable pletnaMonitor;

// volic
void kruzkuj() {

    std::unique_lock<std::mutex> pletnaLock(pletnaMutex);
    while (emptyPletnaCount <= 0) {
        pletnaMonitor.wait(pletnaLock);
    }
    emptyPletnaCount--;
    pletnaLock.unlock();

    std::this_thread::sleep_for(PLETNA_TIME);

    pletnaLock.lock();
    emptyPletnaCount++;
    pletnaLock.unlock();

    pletnaMonitor.notify_all();
}

void vhadzuj() {
    std::unique_lock<std::mutex> urnaLock(urnaMutex);
    while (emptyUrnaCount <= 0) {
        urnaMonitor.wait(urnaLock);
    }
    emptyUrnaCount--;
    urnaLock.unlock();

    std::this_thread::sleep_for(URNA_TIME);

    urnaLock.lock();
    voteCount++;
    emptyUrnaCount++;
    urnaLock.unlock();

    urnaMonitor.notify_all();
}

void volic() {
    kruzkuj();
    vhadzuj();
}

void printVoteCount() {
    while (print) {
        std::cout << voteCount << std::endl;
        std::this_thread::sleep_for(PRINT_PERIOD);
    }
}

// main f.
int main() {

    //
    emptyUrnaCount = URNA_COUNT;
    emptyPletnaCount = PLETNA_COUNT;
    voteCount = 0;

    //
    std::thread volici[POCET_VOLICOV];

    std::thread printingThread(printVoteCount);

    for (int i = 0; i < POCET_VOLICOV; i++) {
        volici[i] = std::thread(volic);
//        std::this_thread::sleep_for(VOTER_TIME);
    }

    for (auto &volic : volici) {
        volic.join();
    }

    print = false;

    exit(EXIT_SUCCESS);
}