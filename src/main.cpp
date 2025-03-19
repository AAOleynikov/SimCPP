#include <fstream>
#include "SimCPP.h"

int main() {
    unsigned int R1 = 6; 
    unsigned int RGB1 = 26;
    unsigned int RGB2 = 24;
    unsigned int RGB3G1 = 30;
    unsigned int RGB3B1 = 27;

    std::ofstream trLog; trLog.open("logs\\transactLog.txt",std::ios::trunc);
    std::ofstream CFECLog; CFECLog.open("logs\\CFECLog.txt",std::ios::trunc);
    std::ofstream sysEvLog; sysEvLog.open("logs\\sysEvLog.txt",std::ios::trunc);
    std::ofstream statEvLog; statEvLog.open("logs\\statEvLog.txt",std::ios::trunc);

    SimCPP mySim1("three grhoups of workers", R1, RGB1, RGB2, RGB3G1, RGB3B1);
    mySim1.storage("workers_1",1);
    mySim1.storage("workers_2",2);
    mySim1.storage("workers_3",3);

    mySim1.start(1,&sysEvLog,&statEvLog,&trLog,&CFECLog);

    mySim1.initGenerate(1,6);
    mySim1.initGenerate(7,300);

    while(mySim1.isRunning()) {
        switch(mySim1.sysEvent()) {
            case 0: mySim1.generate(6);
            case 1: mySim1.stage1(); break; // создание транзакта, регистрация его в очереди QUEUE_1 и попытка занять рабочих 1й или 3й группы
            // в случае успешного занятия устройства, удаленеи его из очереди QUEUE_1
            case 2: mySim1.stage2(); break; // освобождение рабочих 1й группы
            case 3: mySim1.stage3(); break; // освобождение рабочих 3й группы
            case 4: mySim1.stage4(); break; // регистрация в очереди QUEUE_2 и попытка занять рабочих 2й или 3й группы
            // в случае успешного занятия устройства, удаленеи его из очереди QUEUE_2
            case 5: mySim1.stage5(); break; // освобождение рабочих 2й группы
            case 6: mySim1.stage6(); break; // освобождение рабочих 3й группы
            case 7: mySim1.terminate(1); break; // завершение модели
            default: break;
        }
    }

    trLog.close();
    CFECLog.close();
    sysEvLog.close();
    statEvLog.close();
}