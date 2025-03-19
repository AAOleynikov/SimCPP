#pragma once

#include "Transact.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

class SimLogs {

    friend class SimCPP;
    private:

        class Logs {
            private:
                std::ofstream* _fLog;
            public:
                Logs(std::ofstream* fLog = nullptr): _fLog (fLog) {};
                void logMess (std::string message, bool framing);
                bool isOpen();
        };

        Logs _sysEvLog;    //sys events log
        Logs _statLog;     //statistics
        Logs _transactLog; //transact log
        Logs _CFECLog;     //CEC and FEC log

        SimLogs (std::ofstream* sysEvLog, std::ofstream* statLog,  std::ofstream* transactLog, std::ofstream* CFECLog): \
            _sysEvLog(sysEvLog), _statLog(statLog), _transactLog(transactLog), _CFECLog(CFECLog) {}
    public:

        bool isEnable_SysEvLog() { return _sysEvLog.isOpen(); }
        bool isEnable_StatLog() { return _statLog.isOpen(); }
        bool isEnable_transactLog() { return _transactLog.isOpen(); }
        bool isEnable_CFECLog() { return _CFECLog.isOpen(); }

        void logMess_sysEvLog(std::string message, bool framing = false) { _sysEvLog.logMess(message, framing); }
        void logMess_statLog(std::string message, bool framing = false) { _statLog.logMess(message, framing); }
        void logMess_transactLog(std::string message, bool framing = false) { _transactLog.logMess(message, framing); }
        void logMess_CFECLog(std::string message, bool framing = false) { _CFECLog.logMess(message, framing); }

        void modelInitMess(std::string modelName);
        void modelEndMess(const std::string reasonOfEnding);
};

bool SimLogs::Logs::isOpen() {
    if (_fLog != nullptr)
        return true;
    return false;
}

void SimLogs::Logs::logMess (std::string message, bool framing) {
    if (!this->isOpen())
        throw std::logic_error("You cannot send log messages until a file stream is assigned");
    std::time_t t = std::time(0);   
    std::tm* now = std::localtime(&t);
    if (*_fLog) {
        // time + message
        std::string out { std::to_string(now->tm_year + 1900) + '-' + std::to_string(now->tm_mon + 1) + '-' + std::to_string(now->tm_mday) + ' ' \
                          + std::to_string(now->tm_hour) + ':' + std::to_string(now->tm_min) + ':' + std::to_string(now->tm_sec) + '\t' + message };     
        if (framing) {
            std::string frame = {"==--+"}; 
            out = frame + '\n' + out + '\n' + frame;
        }
        *_fLog <<out<<std::endl;
        if (_fLog->bad()) {
            std::cout<<"Error when trying to write to the logging file. Logging mode will be disabled, but the execution of the model will continue."<<std::endl;
            _fLog = nullptr;
        }
    }
    else {
        std::cout<<"Error when trying to write to the logging file. Logging mode will be disabled, but the execution of the model will continue."<<std::endl;
        _fLog = nullptr;
    }
}

void SimLogs::modelInitMess(std::string modelName) {
    std::string message;
    if (_sysEvLog.isOpen()) {
        message = {"\n\"" + modelName + "\" Sim model system events logging enabled\n"};

        if (_statLog.isOpen())
            message += "Sim model statistics logging enabled\n";
        else
            message += "Sim model statistics logging disabled\n";

        if (_transactLog.isOpen())
            message += "Sim model transacts logging enabled\n";
        else
            message += "Sim model transacts logging disabled\n";

        if (_CFECLog.isOpen())
            message += "Sim model CEC and FEC logging enabled\n";
        else
            message += "Sim model CEC and FEC logging disabled\n";

        message += "Transact tuple " + Transact::getTransactMeaningString();
        _sysEvLog.logMess(message, true);
    }

    if (_statLog.isOpen()) {
        message = "\n\"" + modelName + "\" Sim model statistics logging enabled";
        _statLog.logMess(message, true);
    }

    if (_CFECLog.isOpen()) {
        message = "\n\"" + modelName + "\" Sim model CEC and FEC logging enabled\nTransact tuple " + Transact::getTransactMeaningString();
        _CFECLog.logMess(message, true);
    }

    if (_transactLog.isOpen()) {
        message = "\n\"" + modelName + "\" Sim model transactions trail logging enabled\nTransact tuple " + Transact::getTransactMeaningString();
        _transactLog.logMess(message, true);
    }
}

void SimLogs::modelEndMess(const std::string reasonOfEnding) {
    std::string message;

    if (_sysEvLog.isOpen()) {
        message = "\nSim model system events logging disabled\n" + reasonOfEnding;
        _statLog.logMess(message, true);
    }

    if (_statLog.isOpen()) {
        message = "\nSim model statistics logging disabled\n" + reasonOfEnding;
        _sysEvLog.logMess(message, true);
    }

    if (_CFECLog.isOpen()) {
        message = "\nSim model CEC and FEC logging disabled\n" + reasonOfEnding;
        _CFECLog.logMess(message, true);
    }

    if (_transactLog.isOpen()) {
        message = "\nSim model transactions trail logging disabled\n" + reasonOfEnding;
        _transactLog.logMess(message, true);
    }
}