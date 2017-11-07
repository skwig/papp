/*
Meno:
Datum:
 
Simulujte nasledujúcu situáciu. V istej firme je 7 programátorov a 5 testerov. Testeri sa delia na 2 skupiny -
 dvaja rýchli (testujú 2s) a traja pomalí (testujú 3s). Programátori a testeri neustále pracujú na vyvíjanom programe,
 prièom naraz na òom môu pracova jedine programátori alebo testeri, nie spolu, keïe by to pokazilo výsledky.
 
Programovanie trvá programátorovi nejaký èas - v simulácii 3s, prièom potom si dáva 2s prestávku.
 
Testovanie trvá tiez nejaký cas, podla výkonnosti testera - 2s alebo 3s. Testeri si dávajú 3s prestávku v mensích
 skupinkách po troch, po tom, co sa ich nazbiera dostatoèný pocet po testovaní (kazdý tester si musí spravit prestávku,
 kým zacne znova testovat).
 
Cela simulácia nech trvá 30s.
 
1. Doplnte do programu pocítadlo pocítajúce, kolko krát ktorý tester a programátor pracoval na programe. [1b]
 
2. Zabezpecte, aby na programe naraz nepracovali testeri a programátori, pricom clenovia rovnakej skupiny môzu pracovat spolocne.
Ak na zaèatie práce caká uz celá druhá skupina - 7 programátorov, alebo 5 testerov uz dalsí clenovia opacnej skupiny
 nezacínajú prácu na programe. [4b]
 
2. Zabezpecte správne osetrenie prestávky testerov v skupinke po troch. [4b]
 
3. Osetrite v programe správne ukonèenie simulácie po uplynutí stanoveného èasu. [1b]
 
Poznámky:
- na synchronizáciu pouite iba mutexy a podmienené premenné
- nespoliehajte sa na uvedené èasy, simulácia by mala fungova aj s inými èasmi
- build (console): gcc programatori_a_testeri -o programatori_a_testeri -lpthread
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

// TODO: reader&writer, symmetric

using namespace std::chrono_literals;

const auto PROGRAMMING_TIME = 3s;
const auto PROGRAMMING_BREAK_TIME = 2s;
const auto TESTING_TIME_2 = 2s;
const auto TESTING_TIME_3 = 3s;
const auto TESTING_BREAK_TIME = 3s;

const auto TOTAL_TIME = 30s;

const int PROGRAMMER_COUNT = 7;
const int TESTER_COUNT = 5;

const int TESTER_BREAK_GROUP_SIZE = 3;

//pocitadlo pre testerov
int pocitadlo_testerov[5] = {0};

//pocitadlo pre programatorov
int pocitadlo_programatorov[7] = {0};

// signal na zastavenie simulacie
bool stoj = false;

int programmerCount = 0;
int testerCount = 0;

int waitingProgrammerCount = 0;
int waitingTesterCount = 0;

std::mutex totalTesterCounterMutex;
std::mutex totalProgrammerCounterMutex;

std::mutex workMutex;

std::condition_variable noProgrammerCond;
std::condition_variable noTesterCond;

std::mutex testerBarrierMutex;
std::condition_variable testerBarrierCond;
std::condition_variable noBreakCond;

bool inTesterBarrier = false;

int testerBreakCount = 0;

// programovanie
void programovanie() {
    std::this_thread::sleep_for(PROGRAMMING_TIME);
}

// prestavka programatora
void prestavka_programator() {
    std::this_thread::sleep_for(PROGRAMMING_BREAK_TIME);
}

// testovanie
void testovanie(int i) {
    std::this_thread::sleep_for(i < 2 ? TESTING_TIME_2 : TESTING_TIME_3);
}

// prestavka testera
void prestavka_tester(int i) {
    std::this_thread::sleep_for(TESTING_BREAK_TIME);
}

// programator
void programator(int id) {

    int cislo_programatora = id;

    // pokial nie je zastaveny
    while (!stoj) {
        std::unique_lock<std::mutex> workLock(workMutex);

        while (!stoj && (testerCount != 0 || waitingTesterCount == TESTER_COUNT)) {
//            std::cout<<"Programator " << cislo_programatora << " caka" << std::endl;
            waitingProgrammerCount++;
            noTesterCond.wait(workLock);
            waitingProgrammerCount--;
//            std::cout<<"Programator " << cislo_programatora << " prestava cakat" << std::endl;
        }

        if (stoj) {
            break;
        }

        programmerCount++;

        workLock.unlock();

        //
        if (!stoj) {
//            std::cout<<"Programator " << cislo_programatora << " zacina pracovat" << std::endl;
            programovanie();
            std::unique_lock<std::mutex> counterLock(totalProgrammerCounterMutex);
            pocitadlo_programatorov[cislo_programatora]++;
//            std::cout<<"Programator " << cislo_programatora << " prestava pracovat" << std::endl;
            counterLock.unlock();
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

// tester
void tester(int id) {

    // priradenie cislo testera
    int cislo_testera = id;

    // pokial nie je zastaveny
    while (!stoj) {


        std::unique_lock<std::mutex> workLock(workMutex);

        while (!stoj && (programmerCount != 0 || waitingProgrammerCount == PROGRAMMER_COUNT)) {
//            std::cout<<"Tester " << cislo_testera << " caka" << std::endl;
            waitingTesterCount++;
            noProgrammerCond.wait(workLock);
            waitingTesterCount--;
//            std::cout<<"Tester " << cislo_testera << " prestava cakat" << std::endl;
        }

        if (stoj) {
            break;
        }

        testerCount++;

        workLock.unlock();

        if (!stoj) {
//            std::cout<<"Tester " << cislo_testera << " zacina pracovat" << std::endl;
            testovanie(cislo_testera);
            std::unique_lock<std::mutex> counterLock(totalTesterCounterMutex);
            pocitadlo_testerov[cislo_testera]++;
//            std::cout<<"Tester " << cislo_testera << " prestava pracovat" << std::endl;
            counterLock.unlock();
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
        std::unique_lock<std::mutex> breakLock(testerBarrierMutex);

        // pockaj kym je bariera prazdna
        noBreakCond.wait(breakLock, [] { return !inTesterBarrier || stoj; });

        if (stoj) {
            break;
        }

        // in
        testerBreakCount++;
        if (testerBreakCount != TESTER_BREAK_GROUP_SIZE) {
            testerBarrierCond.wait(breakLock, [] { return inTesterBarrier || stoj; });
        } else {
            inTesterBarrier = true;
            testerBarrierCond.notify_all();
        }

        if (stoj) {
            break;
        }

        // out
        testerBreakCount--;
        if (testerBreakCount != 0) {
            testerBarrierCond.wait(breakLock, [] { return !inTesterBarrier || stoj; });
        } else {
            inTesterBarrier = false;
            testerBarrierCond.notify_all();
            noBreakCond.notify_all();
        }

        if (stoj) {
            break;
        }

        breakLock.unlock();

        //
        prestavka_tester(cislo_testera);
    }
}

int main() {
    int i;

    std::thread programatori[PROGRAMMER_COUNT];
    std::thread testeri[TESTER_COUNT];

    for (i = 0; i < PROGRAMMER_COUNT; ++i) {
        programatori[i] = std::thread(programator, i);
    }

    for (i = 0; i < TESTER_COUNT; ++i) {
        testeri[i] = std::thread(tester, i);
    }

    std::this_thread::sleep_for(TOTAL_TIME);
    stoj = true;

    noProgrammerCond.notify_all();
    noTesterCond.notify_all();

    testerBarrierCond.notify_all();
    noBreakCond.notify_all();


    std::cout << "Koniec simulacie" << std::endl;

    for (i = 0; i < PROGRAMMER_COUNT; ++i) {
        std::cout << "Programmer " << i << std::endl;
        programatori[i].join();
    }

    for (i = 0; i < TESTER_COUNT; ++i) {
        std::cout << "Tester " << i << std::endl;
        testeri[i].join();
    }

    printf("/---------------------------------/\n");

    for (i = 0; i < PROGRAMMER_COUNT; i++) {
        printf("Programator cislo %d na projekte pracoval %d krat\n", i, pocitadlo_programatorov[i]);
    }

    printf("/---------------------------------/\n");

    for (i = 0; i < TESTER_COUNT; i++) {
        printf("Tester cislo %d na projekte pracoval %d krat\n", i, pocitadlo_testerov[i]);
    }

    printf("/---------------------------------/\n");

    exit(EXIT_SUCCESS);
}