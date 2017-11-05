/*
Meno:
Datum: 24.10.2016
Simulujte nasledujucu situaciu. Desiati pracovnici firmy pripravuju a odosielaju baliky. Styria z nich -
balici, balia zasielky a umiestnuju ich do priestoru na odoslanie.
Balenie im trva nejajky cas (v simulacii 2s). Dalsi styria su zodpovedni za odoslanie zabalenych zasielok -
odosielatelia.
Odosielatelia beru z priestoru baliky a odosielaju ich. Odosielanie tiez trva nejaky cas (v simulacii 3s).
Posledna dvojica su pomocnici a pomahaju odosielatelom v pripade potreby, pricom im odosialnie trva tiez
nejaky cas (v simulacii 1s).
Cela simulacia nech trva 30s.
1. Zabezpecte, aby maximalny pocet zabalenych balikov cakajucich na odoslanie nepresiahol 10,
teda v pripade, ze vyhradeny priestor pre baliky je naplneny, balici cakaju, kym sa uvolni miesto [4b]
2  Zabezpecte, aby pomocnici boli aktivovani iba v pripade, ze pocet cakajucich balikov cakajucich
na odoslanie presiahne 8. Nasledne su deaktivovani v pripade, ze pocet balikov klesne pod 5. [3b]
3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu tak,
aby nikto po stanovenom case uz nezacal dalsiu cinnost. [2b]
4. Doplnte do programu vypisy na obrazovku, ktore budu obsahovat cislo pracovnika, teda 1 az N,
jeho typ, teda Balic, Odosielatel, alebo Pomocnik
a informaciu o zaciatku, alebo konci urcitej cinnosti - teda napriklad "Balic c. 3 zacina balit".
Po ukonceni programu vypiste, kolko balikov bolo celkovo zabalenych a kolko odoslanych odosielatelmi
a kolko pomocnikmi. [1b]
Poznamky:
- priestor pre baliky reprezentujte napriklad polom cisel, kde mozete vkladat poradove cislo balika
- na synchronizaciu pouzite iba mutexy + podmienene premenne
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc balici.c -o balici -lpthread
*/

#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>

using namespace std::chrono_literals;

char stol[10] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
int pozicia_na_umiestnenie = 0;
int pozicia_na_zobratie = 0;

bool run;

const auto PACK_TIME = 2s;
const auto SEND_TIME = 3s;
const auto HELPER_SEND_TIME = 1s;
const auto TOTAL_TIME = 30s;

const int TESTER_COUNT = 10;
const int GENERATOR_COUNT = 4;

const int TABLE_SIZE = 10;

int tableCount;

int totalGeneratedCount;
int totalTestedCount;

std::mutex tableMutex;
std::mutex helperMutex;

std::condition_variable emptyTableMonitor;
std::condition_variable fullTableMonitor;
std::condition_variable helperMonitor;


bool stop = false;

bool helpersEnabled = false;

//balic
void zabal() {
    std::this_thread::sleep_for(PACK_TIME);
}

//odosielatel
void odosli() {
    std::this_thread::sleep_for(SEND_TIME);
}

//pomocnik
void odosli_pomocnik() {
    std::this_thread::sleep_for(HELPER_SEND_TIME);
}

void balic(int id) {
    while (!stop) {
        std::cout << "Balic c." << id << " zacina balit." << std::endl;

        zabal();

        std::unique_lock<std::mutex> tableLock(tableMutex);
        while (tableCount == TABLE_SIZE) {
            fullTableMonitor.wait(tableLock);
        }
        tableCount++;

        if (tableCount > 8) {
            helpersEnabled = true;
            helperMonitor.notify_all();
        }

        tableLock.unlock();

        emptyTableMonitor.notify_one();
        helperMonitor.notify_one();
        std::cout << "Balic c." << id << " konci balit." << std::endl;
    }
}

void odosielatel(int id) {
    while (!stop) {

        std::unique_lock<std::mutex> tableLock(tableMutex);
        std::cout << "Odosielatel c." << id << " zacina odosielat." << std::endl;
        while (tableCount == 0) {
            emptyTableMonitor.wait(tableLock);
        }
        tableCount--;
        tableLock.unlock();

        fullTableMonitor.notify_one();

        odosli();
        std::cout << "Odosielatel c." << id << " konci odosielat." << std::endl;
    }
}

void pomocnik(int id) {
    while (!stop) {

        std::unique_lock<std::mutex> tableLock(tableMutex);
        std::cout << "Pomocnik c." << id << " zacina pomahat & odosielat." << std::endl;
        while (!helpersEnabled) {
            helperMonitor.wait(tableLock);
        }
        tableCount--;

        if (tableCount < 5) {
            helpersEnabled = false;
        }

        tableLock.unlock();

        fullTableMonitor.notify_one();

        odosli_pomocnik();
        std::cout << "Pomocnik c." << id << " konci pomahat & odosielat." << std::endl;
    }
}

int main() {
    int i;

    //
    helpersEnabled = false;
    tableCount = 0;

    std::thread balici[4];
    std::thread odosielatelia[4];
    std::thread pomocnici[2];

    for (i = 0; i < 4; i++) {
        balici[i] = std::thread(balic, i + 1);
    }

    for (i = 0; i < 4; i++) {
        odosielatelia[i] = std::thread(odosielatel, i + 1);
    }

    for (i = 0; i < 2; i++) {
        pomocnici[i] = std::thread(pomocnik, i + 1);
    }

    std::this_thread::sleep_for(TOTAL_TIME);

    stop = true;

    std::cout << "---- Simulacia konci ----\n";

    for (i = 0; i < 4; i++) {
        balici[i].join();
    }

    for (i = 0; i < 4; i++) {
        odosielatelia[i].join();
    }

    for (i = 0; i < 2; i++) {
        pomocnici[i].join();
    }

    exit(EXIT_SUCCESS);
}