#include <fstream>
#include <C:\Users\666an\Documents\BMSTU\3SEM\Pract_C++\Practica_3\SimCPP\SimCPP.h>


#define METKA1 5
#define METKA2 8
#define METKA3 14
#define METKA4 20
#define METKA5 24
#define METKA6 27
#define METKA7 33

int main() {
    unsigned int R1 = 6;
    unsigned int RGB1 = 26;
    unsigned int RGB2 = 24;
    unsigned int RGB3G1 = 30;
    unsigned int RGB3B1 = 27;

    std::ofstream trLog;
    std::ofstream CFECLog;
    std::ofstream sysEvLog;
    std::ofstream statEvLog;

    CFECLog.open("logs\\CFECLog.txt",std::ios::trunc);
    sysEvLog.open("logs\\sysEvLog.txt",std::ios::trunc);
    trLog.open("logs\\transactLog.txt",std::ios::trunc);
    statEvLog.open("logs\\statEvLog.txt",std::ios::trunc);

    SimCPP mySim1("three grhoups of workers");
    mySim1.storage("workers_1",3);
    mySim1.storage("workers_2",3);
    mySim1.storage("workers_3",3);

    mySim1.start(1,&sysEvLog,&statEvLog,&trLog,&CFECLog);
    //mySim1.start(1,&sysEvLog,&statEvLog,nullptr,nullptr);

    mySim1.initGenerate(1,6);
    mySim1.initGenerate(40,3600);

    while(mySim1.isRunning()) {
        switch(mySim1.sysEvent()) {
            case 1: mySim1.generate(mySim1.exponential(R1)); break;

            case 2: mySim1.queue("W1_QUEUE"); break;
            case 3: mySim1.test(mySim1.getLinkParam("q_workers_1","CH") != 0, METKA1); break;
            case 4: mySim1.link("q_workers_1", "M1"); break;

            case 5: mySim1.test(mySim1.getStorageParam("workers_1","R") == 0, METKA2); break; //METKA1
            case 6: mySim1.test( ((mySim1.getStorageParam("workers_3","R") != 0) && \
                (mySim1.getLinkParam("q_workers_1","CH")) >= mySim1.getLinkParam("q_workers_2","CH")) != true, METKA3); break;
            case 7: mySim1.link("q_workers_1", "M1"); break;

            case 8: mySim1.enter("workers_1"); break; //METKA2
            case 9: mySim1.depart("W1_QUEUE"); break;
            case 10:mySim1.advance(mySim1.exponential(RGB1)); break;
            case 11:mySim1.leave("workers_1"); break;
            case 12:mySim1.unlink("q_workers_1", METKA1, 1); break;
            case 13:mySim1.transfer(METKA4); break;

            case 14:mySim1.enter("workers_3"); break; //METKA3
            case 15:mySim1.depart("W1_QUEUE"); break;
            case 16:mySim1.advance(mySim1.exponential(RGB3G1)); break;
            case 17:mySim1.leave("workers_3"); break;
            case 18:mySim1.unlink("q_workers_1", METKA1, 1); break;
            case 19:mySim1.unlink("q_workers_2", METKA5, 1); break;

            case 20:mySim1.queue("W2_QUEUE"); break; //METKA4
            case 21:mySim1.assign("time", mySim1.getModelTime()); break;
            case 22:mySim1.test(mySim1.getLinkParam("q_workers_2","CH") != 0, METKA5); break;
            case 23:mySim1.link("q_workers_2", "time"); break;

            case 24: mySim1.test(mySim1.getStorageParam("workers_2","R") == 0, METKA6); break; //METKA5
            case 25: mySim1.test( ((mySim1.getStorageParam("workers_3","R") != 0) && \
                (mySim1.getLinkParam("q_workers_2","CH")) >= mySim1.getLinkParam("q_workers_1","CH")) != true, METKA7); break;
            case 26: mySim1.link("q_workers_2", "time"); break;

            case 27: mySim1.enter("workers_2"); break; //METKA6
            case 28: mySim1.depart("W2_QUEUE"); break;
            case 29:mySim1.advance(mySim1.exponential(RGB2)); break;
            case 30:mySim1.leave("workers_2"); break;
            case 31:mySim1.unlink("q_workers_2", METKA5, 1); break;
            case 32:mySim1.terminate(); break;

            case 33:mySim1.enter("workers_3"); break; //METKA7
            case 34:mySim1.depart("W2_QUEUE"); break;
            case 35:mySim1.advance(mySim1.exponential(RGB3B1)); break;
            case 36:mySim1.leave("workers_3"); break;
            case 37:mySim1.unlink("q_workers_1", METKA1, 1); break;
            case 38:mySim1.unlink("q_workers_2", METKA5, 1); break;
            case 39:mySim1.terminate(); break;

            case 40:mySim1.terminate(10000); break;
            case 41:mySim1.terminate(1); break;

            default: break;
        }
    }

    trLog.close();
    CFECLog.close();
    sysEvLog.close();
    statEvLog.close();
}