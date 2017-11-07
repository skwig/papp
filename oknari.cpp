/*
Meno:
Datum:

Simulujte nasledujucu situaciu. Piati oknari osadzaju okna. Na osadenie maleho okna su potrebni dvaja oknari a osadzaju
 ho nejaky cas (v simulacii 2s) a na osadenie velkeho okna su potrebni 3 oknari a osadzaju ho dlhsi cas (v simulacii 3s).
 Okna su naskadane v kamione v takomto poradi (V - velke 15x, M - male 15x): "VMMVVMMMVVVVVMMMVVVVMMMVVVMMMM".
 Simulacia konci, ked su vsetky okna osadene.

1. Doplnte do programu vypis na obrazovku v ktorom bude cislo robotnika 1 az N, cinnost, a ci cinnost zacina alebo konci,
 napr. "robotnik c.2 osad_male zaciatok". Doplnte do programu pocitadla pre male a velke okna, vzdy ked je okno osadene,
 inkrementujte zodpovedajuce pocitadlo. [3b]

2. Zabezpecte, aby podla velkosti spravny pocet oknarov osadzal okno spravny cas, ak je to potrebne, musia na seba pockat. [5b]

3. Osetrite v programe spravne ukoncenie simulacie po osadeni vetkych okien. [2b]

Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc oknari.c -o oknari -lpthread
*/

#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <cstring>

using namespace std::chrono_literals;

const auto BIG_TIME = 3s;
const auto SMALL_TIME = 2s;

// okna
char *okna = "VMMVVMMMVVVVVMMMVVVVMMMVVVMMMM";
const auto windowCount = strlen(okna);

const int BIG_WINDOW_WORKERS = 3;
const int SMALL_WINDOW_WORKERS = 2;

int currentWindow = 0;

int barrierCount = 0;

bool inBarrier = false;

int totalBigCounter = 0;
int totalSmallCounter = 0;

std::mutex windowMutex;
std::mutex printMutex;

std::condition_variable windowBarrier;
std::condition_variable windowQueue;

bool stoj = false;

// signal na zastavenie simulacie

// oknar
void osad_male() {
    std::this_thread::sleep_for(SMALL_TIME);
}

void osad_velke() {
    std::this_thread::sleep_for(BIG_TIME);
}

void oknar(int id) {

    while (true) {

        std::unique_lock<std::mutex> windowLock(windowMutex);
        windowQueue.wait(windowLock, [] { return !inBarrier; });

        if (currentWindow >= windowCount) {
            break;
        }

        const bool isBig = (okna[currentWindow] == 'V');
        const int workersNeeded = isBig ? BIG_WINDOW_WORKERS : SMALL_WINDOW_WORKERS;

        // in
        barrierCount++;
        if (barrierCount != workersNeeded) {
            while (!inBarrier && !stoj) { windowBarrier.wait(windowLock); }
        } else {
            inBarrier = true;
            windowBarrier.notify_all();
        }

        // out
        barrierCount--;
        if (barrierCount != 0) {
            while (inBarrier && !stoj) { windowBarrier.wait(windowLock); }
        } else {
            inBarrier = false;
            windowQueue.notify_all();
            windowBarrier.notify_all();

            currentWindow++;

            if (isBig) {
                totalBigCounter++;
            } else {
                totalSmallCounter++;
            }
        }

        //
        windowLock.unlock();


        if (isBig) {
            std::unique_lock<std::mutex> lock(printMutex);
            std::cout << "Robotnik " << id << " zacina osadzat VELKE okno." << std::endl;
            lock.unlock();

            osad_velke();

            lock.lock();
            std::cout << "Robotnik " << id << " prestava osadzat VELKE okno." << std::endl;
            lock.unlock();
        } else {
            std::unique_lock<std::mutex> lock(printMutex);
            std::cout << "Robotnik " << id << " zacina osadzat MALE okno." << std::endl;
            lock.unlock();

            osad_male();

            lock.lock();
            std::cout << "Robotnik " << id << " prestava osadzat MALE okno." << std::endl;
            lock.unlock();
        }
    }
}

int main() {
    int i;

    std::thread oknari[5];

    for (i = 0; i < 5; i++) {
        oknari[i] = std::thread(oknar, i);
    }

    // zmenit, simulacia koci ked su okna osadene

    for (i = 0; i < 5; i++) {
        oknari[i].join();
    }

    std::cout << "Celkovy pocet osadeni velkych oken: " << totalBigCounter << std::endl;
    std::cout << "Celkovy pocet osadeni malych oken: " << totalSmallCounter << std::endl;


    exit(EXIT_SUCCESS);
}