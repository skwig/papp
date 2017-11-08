/*
Meno: Matej Brečka
Datum: 8.11.2017

 Simulujte nasledujucu situaciu. Vo firme pracuje 7 programatorov a 5 testerov (3 pomali a 2 rychli). Programovanie
 trva programatorovi nejaky cas (v simululacii 3s) a potom si da prestavku (v simulacii 2s); a potom zase ide programator
 programovat. Tester nejaky cas testuje (pomaly 3s a rychly 2s) a potom si tiez dava prestavku (3s); po skonceni ktorej
 ide zase testovat. Cela simulacia nech trva 30s.

1. Doplnte do programu pocitadlo pocitajuce, kolko krat bolo pocas simulacie vykonavane testovanie a kolko krat
 programovanie. [1b]

2. Zabezpecte, aby na programe nepracovali testeri a programatori naraz, cize na programe mozu pracovat alebo iba
 programatori alebo iba testeri. Ani jedna skupina nema prioritu. [4b]

3. Zabezpecte, aby si testeri davali prestavku po skupinach (aby sa pockali v ramci skupiny), vzdy pomali ako jedna
 skupina a rychli ako druha skupina. [4b]

4. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu. [1b]

Poznamky:
- na synchronizaciu pouzite iba mutexy a podmienene premenne
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc programatori_a_testeri_2.c -o programatori_a_testeri_2 -lpthread
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std::chrono_literals;

const auto PROGRAMMING_TIME = 3s;
const auto PROGRAMMING_BREAK_TIME = 2s;
const auto TESTING_TIME_FAST = 2s;
const auto TESTING_TIME_SLOW = 3s;
const auto TESTING_BREAK_TIME = 3s;

const auto TOTAL_TIME = 30s;

const int PROGRAMMER_COUNT = 7;
const int SLOW_TESTER_COUNT = 2;
const int FAST_TESTER_COUNT = 3;

int totalTestedCount = 0;
int totalProgrammedCount = 0;

int programmerCount = 0;
int testerCount = 0;

std::mutex totalTesterCounterMutex;
std::mutex totalProgrammerCounterMutex;

std::mutex workMutex;

std::condition_variable noProgrammerCond;
std::condition_variable noTesterCond;

bool inSlowBarrier = false;
bool inFastBarrier = false;

int slowTesterBarrierCount = 0;
int fastTesterBarrierCount = 0;

std::mutex slowTesterBarrierMutex;
std::mutex fastTesterBarrierMutex;

std::condition_variable slowTesterBarrierCond;
std::condition_variable fastTesterBarrierCond;

// signal na zastavenie simulacie
bool stoj = false;

// programovanie
void programovanie() {
    std::this_thread::sleep_for(PROGRAMMING_TIME);

    std::unique_lock<std::mutex> lock(totalProgrammerCounterMutex);
    totalProgrammedCount++;
    lock.unlock();
}

// prestavka programatora
void prestavka_programator() {
    std::this_thread::sleep_for(PROGRAMMING_BREAK_TIME);
}

// testovanie
void testovanie_pomaly() {
    std::this_thread::sleep_for(TESTING_TIME_SLOW);

    std::unique_lock<std::mutex> lock(totalTesterCounterMutex);
    totalTestedCount++;
    lock.unlock();
}

void testovanie_rychly() {
    std::this_thread::sleep_for(TESTING_TIME_FAST);

    std::unique_lock<std::mutex> lock(totalTesterCounterMutex);
    totalTestedCount++;
    lock.unlock();
}

// prestavka testera
void prestavka_tester() {
    std::this_thread::sleep_for(TESTING_BREAK_TIME);
}

// programator
void programator() {

    while (!stoj) {

        std::unique_lock<std::mutex> workLock(workMutex);

        while (!stoj && (testerCount != 0)) {
            noTesterCond.wait(workLock);
        }

        if (stoj) {
            break;
        }

        programmerCount++;

        workLock.unlock();

        //
        if (!stoj) {
            programovanie();
        }

        //
        workLock.lock();
        programmerCount--;
        if (testerCount == 0) {
            noProgrammerCond.notify_all();
        }
        workLock.unlock();

        if (stoj) {
            break;
        }

        prestavka_programator();

    }
}

// tester rychly
void tester_rychly() {

    while (!stoj) {
        std::unique_lock<std::mutex> workLock(workMutex);

        while (!stoj && (programmerCount != 0)) {
            noProgrammerCond.wait(workLock);
        }

        if (stoj) {
            break;
        }

        testerCount++;

        workLock.unlock();

        if (!stoj) {
            testovanie_rychly();
        }

        workLock.lock();
        testerCount--;
        if (programmerCount == 0) {
            noTesterCond.notify_all();
        }
        workLock.unlock();

        if (stoj) {
            break;
        }


        // prestavka
        std::unique_lock<std::mutex> breakLock(fastTesterBarrierMutex);

        if (stoj) {
            break;
        }

        // in
        fastTesterBarrierCount++;
        if (fastTesterBarrierCount != FAST_TESTER_COUNT) {
            fastTesterBarrierCond.wait(breakLock, [] { return inFastBarrier || stoj; });
        } else {
            inFastBarrier = true;
            fastTesterBarrierCond.notify_all();
        }

        if (stoj) {
            break;
        }

        // out
        fastTesterBarrierCount--;
        if (fastTesterBarrierCount != 0) {
            fastTesterBarrierCond.wait(breakLock, [] { return !inFastBarrier || stoj; });
        } else {
            inFastBarrier = false;
            fastTesterBarrierCond.notify_all();
        }

        if (stoj) {
            break;
        }

        breakLock.unlock();

        //
        prestavka_tester();
    }
}

// tester pomaly
void tester_pomaly() {

    while (!stoj) {
        std::unique_lock<std::mutex> workLock(workMutex);

        while (!stoj && (programmerCount != 0)) {
            noProgrammerCond.wait(workLock);
        }

        if (stoj) {
            break;
        }

        testerCount++;

        workLock.unlock();

        if (!stoj) {
            testovanie_pomaly();
        }

        workLock.lock();
        testerCount--;
        if (programmerCount == 0) {
            noTesterCond.notify_all();
        }
        workLock.unlock();

        if (stoj) {
            break;
        }


        // prestavka
        std::unique_lock<std::mutex> breakLock(slowTesterBarrierMutex);

        if (stoj) {
            break;
        }

        // in
        slowTesterBarrierCount++;
        if (slowTesterBarrierCount != SLOW_TESTER_COUNT) {
            slowTesterBarrierCond.wait(breakLock, [] { return inSlowBarrier || stoj; });
        } else {
            inSlowBarrier = true;
            slowTesterBarrierCond.notify_all();
        }

        if (stoj) {
            break;
        }

        // out
        slowTesterBarrierCount--;
        if (slowTesterBarrierCount != 0) {
            slowTesterBarrierCond.wait(breakLock, [] { return !inSlowBarrier || stoj; });
        } else {
            inSlowBarrier = false;
            slowTesterBarrierCond.notify_all();
        }

        if (stoj) {
            break;
        }

        breakLock.unlock();

        //
        prestavka_tester();
    }
}

int main() {
    int i;

    std::thread programatori[PROGRAMMER_COUNT];
    std::thread testeri_rychli[FAST_TESTER_COUNT];
    std::thread testeri_pomali[SLOW_TESTER_COUNT];

    for (i = 0; i < PROGRAMMER_COUNT; ++i) {
        programatori[i] = std::thread(programator);
    }

    for (i = 0; i < FAST_TESTER_COUNT; ++i) {
        testeri_rychli[i] = std::thread(tester_rychly);
    }

    for (i = 0; i < SLOW_TESTER_COUNT; ++i) {
        testeri_pomali[i] = std::thread(tester_pomaly);
    }


    std::this_thread::sleep_for(TOTAL_TIME);

    std::cout << "Simulacia konci" << std::endl;

    stoj = true;

    for (i = 0; i < PROGRAMMER_COUNT; ++i) {
        programatori[i].join();
    }

    for (i = 0; i < FAST_TESTER_COUNT; ++i) {
        testeri_rychli[i].join();
    }

    for (i = 0; i < SLOW_TESTER_COUNT; ++i) {
        testeri_pomali[i].join();
    }

    std::cout << "Celkovy pocet testovani: " << totalTestedCount << std::endl;
    std::cout << "Celkovy pocet programovani: " << totalProgrammedCount << std::endl;

    exit(EXIT_SUCCESS);
}