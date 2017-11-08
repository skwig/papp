/*
Meno:
Datum:

Simulujte nasledujucu situaciu. Vo firme pracuje 6 grafikov a 4 programatori, ktori spolu vytvaraju webove stranky.
 Kazdy grafik nejaky cas sam vytvara dizajn stranky (v simulacii 2s), potom dizajn umiestni na server a zase ide
 vytvarat dalsi dizajn. Programator si zoberie dizajn zo servera a sam podla neho programuje stranku (v simulacii 4s)
 az kym nie je hotova, a potom si ide zase zobrat dalsi dizajn. Na serveri je miesto iba pre 20 dizajnv,
 simulacia nech trva 30s.

1. Doplnte do programu pocitadlo vsetkych vytvorenych webovych stranok. [1b]

2. Zabezpecte spravnu pracu so serverom; grafik po vytvoreni dizajnu musi pockat, ak je server zaplneny; a programator
 musi pockat, kym nie je na server umiestneny nejaky dizajn. [4b]

3. Ak je server zaplneny, firma povola 4 externych programatorov, kori programuju stranky rychlejsie (v simulacii 1s),
 ale su aj drahsi, a preto ak je na serveri iba 10 dizajnov firma ich uvolni z prace. [4b]

4. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu. [1b]

Poznamky:
- na synchronizaciu pouzite iba mutexy a podmienene premenne
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc grafici_a_programatori.c -o grafici_a_programatori -lpthread
*/
#include<thread>
#include<iostream>
#include<chrono>
#include<condition_variable>

using namespace std::chrono_literals;

const auto DESIGNING_TIME = 2s;
const auto PROGRAMMING_TIME = 4s;
const auto EXTERNAL_PROGRAMMING_TIME = 1s;

const auto TOTAL_TIME = 30s;

int totalProgrammedSites = 0;
int totalDesignedSites = 0;

std::mutex totalProgrammedMutex;
std::mutex totalDesignedMutex;

std::mutex printMutex;

std::mutex serverMutex;

const int MAX_DESIGNS = 20;

int designCount = 0;

std::condition_variable emptyCond;
std::condition_variable fullCond;
std::condition_variable externalProgrammerCond;

bool helpEnabled = false;


// signal na zastavenie simulacie
bool stoj = false;

// programator
void programovanie_stranky() {
    std::this_thread::sleep_for(PROGRAMMING_TIME);
    std::unique_lock<std::mutex> lock(totalProgrammedMutex);
    totalProgrammedSites++;
    lock.unlock();

}

void externy_programovanie_stranky() {
    std::this_thread::sleep_for(EXTERNAL_PROGRAMMING_TIME);
    std::unique_lock<std::mutex> lock(totalProgrammedMutex);
    totalProgrammedSites++;
    lock.unlock();
}

void programator(int id) {

    while (!stoj) {

        std::unique_lock<std::mutex> serverLock(serverMutex);
        while (designCount == 0) {
            emptyCond.wait(serverLock);
        }

        if (stoj) {
            break;
        }

        designCount--;

        if (designCount <= 10) {
            helpEnabled = false;
        }

        serverLock.unlock();

        fullCond.notify_one();

        programovanie_stranky();
    }

}

void externy_programator(int id) {

    while (!stoj) {

        std::unique_lock<std::mutex> serverLock(serverMutex);
        while (!helpEnabled) {
            emptyCond.wait(serverLock);
        }

        if (stoj) {
            break;
        }

        designCount--;

        if (designCount <= 10) {
            helpEnabled = false;
        }

        serverLock.unlock();

        fullCond.notify_one();

        externy_programovanie_stranky();


    }
}


// grafik
void vytvaranie_dizajnu() {
    std::this_thread::sleep_for(DESIGNING_TIME);
    std::unique_lock<std::mutex> lock(totalDesignedMutex);
    totalDesignedSites++;
    lock.unlock();
}

void grafik(int id) {

    while (!stoj) {
        vytvaranie_dizajnu();

        std::unique_lock<std::mutex> serverLock(serverMutex);
        while (designCount == MAX_DESIGNS) {
            fullCond.wait(serverLock);
        }

        if (stoj) {
            break;
        }

        designCount++;

        if (designCount == MAX_DESIGNS) {
            helpEnabled = true;
            externalProgrammerCond.notify_all();
        }

        serverLock.unlock();

        emptyCond.notify_one();
        externalProgrammerCond.notify_one();
        // external notify one?
    }

}


int main() {
    int i;

    std::thread grafici[6];
    std::thread programatori[4];
    std::thread pomocnici[4];

    for (i = 0; i < 6; i++) grafici[i] = std::thread(grafik, i);
    for (i = 0; i < 4; i++)programatori[i] = std::thread(programator, i);
    for (i = 0; i < 4; i++) pomocnici[i] = std::thread(externy_programator, i);

    std::this_thread::sleep_for(TOTAL_TIME);
    std::cout << "Koniec simulacie" << std::endl;
    stoj = true;
    emptyCond.notify_all();
    fullCond.notify_all();
    externalProgrammerCond.notify_all();

    for (i = 0; i < 6; i++) {
        std::cout << "Grafik " << i << " join" << std::endl;
        grafici[i].join();
    }
    for (i = 0; i < 4; i++) {
        std::cout << "Programator " << i << " join" << std::endl;
        programatori[i].join();
    }
    for (i = 0; i < 4; i++) {
        std::cout << "Pomocnik " << i << " join" << std::endl;
        pomocnici[i].join();
    }

    printf("/--------------------------------------------------/\n");
    printf("Celkovy pocet navrhnutych stranok: %d\n", totalDesignedSites);
    printf("Celkovy pocet naprogramovanych stranok: %d\n", totalProgrammedSites);

    exit(EXIT_SUCCESS);
}