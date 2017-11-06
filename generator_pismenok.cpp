/*
Meno:
Datum:

Simulujte nasledujucu situaciu. Styria generovaci pismenok generuju pismenka, generovanie pismenka trva nejaky cas (1s)
a ked ho vygeneruju, umiestnia ho na stol, kde sa zmesti 10 pismenok. Desiati testovaci beru pismenka zo stola a testuju ich,
testovanie pismenka trva nejaky cas (2s). Cela simulacia nech trva 30s.

1. Doplnte do programu pocitadlo vygenerovanych a pocitadlo otestovanych pismenok, na konci simulacie vypiste hodnoty pocitadiel. [2b]

2. Osetrite v programe pristup k stolu - zmente umiestnovanie a branie pismenok tak, aby nehrozilo, ze generovac "prepise"
 pismenko, ktore nebolo otestovane, a ze testovac otestuje pismenko, ktore nebolo vygenerovane alebo uz bolo otestovane. [5b]

3. Osetrite v programe spravne ukoncenie generovacov a testovacov po uplynuti stanoveneho casu simulacie. [3b]

Poznamky:
- na synchronizaciu pouzite iba mutexy, podmienene premenne alebo semafory
- build (console): gcc generator_pismenok -o generator_pismenok -lpthread
*/

// TODO producer & consumer

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std::chrono_literals;

// stol s pismenkami
char stol[10] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
int pozicia_na_umiestnenie = 0;
int pozicia_na_zobratie = 0;

bool run;

const auto GENERATE_TIME = 1s;
const auto TEST_TIME = 2s;
const auto TOTAL_TIME = 30s;

const int TESTER_COUNT = 10;
const int GENERATOR_COUNT = 4;

const int TABLE_SIZE = 10;

int tableCount;

int totalGeneratedCount;
int totalTestedCount;

std::mutex tableMutex;

std::mutex generateMutex;
std::mutex testMutex;

std::condition_variable emptyTableMonitor;
std::condition_variable fullTableMonitor;


// generovanie pismenka
char generuj_pismenko() {
    std::this_thread::sleep_for(GENERATE_TIME);

    std::unique_lock<std::mutex> generateLock(generateMutex);
    totalGeneratedCount++;
    generateLock.unlock();

    return 'A';
}

// testovanie pismenka
void testuj_pismenko(char pismenko) {
    std::this_thread::sleep_for(TEST_TIME);

    std::unique_lock<std::mutex> testLock(testMutex);
    totalTestedCount++;
    testLock.unlock();
}

// generator pismenok
void generovac_pismenok() {

    // pokial nie je zastaveny
    while (run) {

        // vygenerovanie pismenka
        char pismenko = generuj_pismenko();

        std::unique_lock<std::mutex> tableLock(tableMutex);
        while (tableCount == TABLE_SIZE && run) {
            fullTableMonitor.wait(tableLock);
        }

        if (!run) {
            break;
        }

        // umiestni pismenko na stol
        stol[pozicia_na_umiestnenie] = pismenko;
        tableCount++;

        pozicia_na_umiestnenie = (pozicia_na_umiestnenie + 1) % 10;

        emptyTableMonitor.notify_one();

        tableLock.unlock();
    }
}

// testovac pismenok
void testovac_pismenok() {

    // pokial nie je zastaveny
    while (run) {
        // vzatie pismenka zo stola
        std::unique_lock<std::mutex> tableLock(tableMutex);
        while (tableCount == 0 && run) {
            emptyTableMonitor.wait(tableLock);
        }

        if (!run) {
            break;
        }

        char pismenko = stol[pozicia_na_zobratie];
        tableCount--;

        pozicia_na_zobratie = (pozicia_na_zobratie + 1) % 10;

        fullTableMonitor.notify_one();

        tableLock.unlock();

        // otestovanie pismenka
        testuj_pismenko(pismenko);
    }
}

int main() {

    //
    tableCount = 0;
    totalGeneratedCount = 0;
    totalTestedCount = 0;
    run = true;

    //
    std::thread generovaci[GENERATOR_COUNT];
    std::thread testovaci[TESTER_COUNT];


    for (int i = 0; i < GENERATOR_COUNT; ++i) {
        generovaci[i] = std::thread(generovac_pismenok);
    }

    for (int i = 0; i < TESTER_COUNT; ++i) {
        testovaci[i] = std::thread(testovac_pismenok);
    }

    std::this_thread::sleep_for(TOTAL_TIME);
    std::cout << "Koniec" << std::endl;
    run = false;
    fullTableMonitor.notify_all();
    emptyTableMonitor.notify_all();

    for (auto &gen : generovaci) {
        gen.join();
    }

    for (auto &test : testovaci) {
        test.join();
    }

    std::cout << "Generated: " << totalGeneratedCount << " Tested: " << totalTestedCount << std::endl;

    exit(EXIT_SUCCESS);
}