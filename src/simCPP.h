#pragma once

#include <random>
#include <list>
#include <vector> 
#include <string>
#include <fstream> 
#include <iostream>
#include <stdexcept>
#include <algorithm> //string.replace
#include <functional> //[](){} - lambda func

#include "Transact.h"
#include "EventChain.h"
#include "Storages.h"
#include "SimLogs.h"
#include "Queues.h"

class SimCPP {
    private:
        const std::string _modelName;
        unsigned long _maxId;   //current max ID of Transact
        long double _modelTime; //current model time
        unsigned int _counter;  //analog GPSS START directive argument (START _counter)
        unsigned int _R1, _RGB1, _RGB2, _RGB3G1, _RGB3B1;

        EventChain _FEC; //feature event chain
        EventChain _CEC; //current event chain
        EventChain::iterator _CECIt;

        SimLogs* _simLogs;
        Storages _storages;
        Queues _queues;
    public:
        SimCPP (std::string modelName, unsigned int R1, unsigned int RGB1, unsigned int RGB2, unsigned int RGB3G1, unsigned int RGB3B1): \
            _modelName(modelName), _maxId(1), _modelTime(.0), _counter(0), _R1(R1), _RGB1(RGB1), _RGB2(RGB2), _RGB3G1(RGB3G1), _RGB3B1(RGB3B1),
            _FEC("FEC"), _CEC("CEC"), _simLogs(nullptr) { _CECIt = _CEC.begin(); }

        ~SimCPP() {if (_simLogs != nullptr) {delete _simLogs;};};

        bool isRunning();
        unsigned int sysEvent();
        long double getModelTime();

        void start(unsigned int count, std::ofstream* sysEvLog = nullptr, std::ofstream* statLog = nullptr, \
                  std::ofstream* transactLog = nullptr, std::ofstream* CFECLog = nullptr);
        void storage(const std::string storageName, const unsigned int maxChannels);
        void initGenerate(unsigned int birthState, long double birthTime);
        void generate(long double birthDelayInterval);
        void terminate(unsigned int reduceCounter = 0);

        void queue(const std::string queueName);
        void depart(const std::string queueName);

        void advance(long double delay);
        unsigned int enter(const std::string name, const unsigned int numbOfChannels = 1);
        void leave(const std::string name, const unsigned int numbOfChannels = 1);
        double exponential(double mean);
        
        void stage1 ();
        void stage2 ();
        void stage3 ();
        void stage4 ();
        void stage5 ();
        void stage6 ();
};

//-----

void SimCPP::stage1() {
    this->queue("W1_QUEUE");

    if (_storages.getStorageParam("workers_1","R") == 0 && _storages.getStorageParam("workers_3","R") != 0 && \
        _queues.getQueueParam("W1_QUEUE", "Q") >= _queues.getQueueParam("W2_QUEUE", "Q")) {
            (*_CECIt)->setNextState(3);
            this->enter("workers_3");
            this->depart("W1_QUEUE");
            this->advance(this->exponential(_RGB3G1));
    }
    else if (_storages.getStorageParam("workers_1","R") != 0) {
        this->enter("workers_1");
        (*_CECIt)->setNextState(2);
        this->depart("W1_QUEUE");
        this->advance(this->exponential(_RGB1));
    }
    else {
        _CECIt++;
    }
}

void SimCPP::stage2() {
    this->leave("workers_1");
    (*_CECIt)->setNextState(4);
}

void SimCPP::stage3() {
    this->leave("workers_3");
    (*_CECIt)->setNextState(4); 
}

void SimCPP::stage4() {
    this->queue("W2_QUEUE");

    if (_storages.getStorageParam("workers_2","R") == 0 && _storages.getStorageParam("workers_3","R") != 0 && \
        _queues.getQueueParam("W2_QUEUE", "Q") >= _queues.getQueueParam("W1_QUEUE", "Q")) {
            (*_CECIt)->setNextState(6);
            this->enter("workers_3");
            this->depart("W2_QUEUE");
            this->advance(this->exponential(_RGB3B1));
    }
    else if (_storages.getStorageParam("workers_2","R") != 0) {
        this->enter("workers_2");
        (*_CECIt)->setNextState(5);
        this->depart("W2_QUEUE");
        this->advance(this->exponential(_RGB2));
    }
    else {
        _CECIt++;
    }
}

void SimCPP::stage5 () {
    this->leave("workers_2");
    this->terminate();
}

void SimCPP::stage6 () {
    this->leave("workers_3");
    this->terminate();
}


void SimCPP::queue(const std::string queueName) {
    Transact* currTransact;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    _queues.queue(queueName, currTransact);
}

void SimCPP::depart(const std::string queueName) {
    Transact* currTransact;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    _queues.depart(queueName, currTransact);
}

void SimCPP::advance(long double delay) {
    Transact* currTransact;
    std::string message;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    _CECIt++;
 
    currTransact->setTime(_modelTime + delay);
    _CEC.eraseTrans(currTransact);

    _FEC.emplace(std::find_if(_FEC.begin(),_FEC.end(),[ currTransact ](Transact* transact) {return currTransact->getTime() < transact->getTime();}), currTransact);

    if (_simLogs->isEnable_CFECLog()) {
                message = "\"advance\" Xact:" + std::to_string(currTransact->getID()) + " model time: " + std::to_string(_modelTime) \
                                     + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n'; 
                _simLogs->logMess_CFECLog(message);
        }

    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string(currTransact->getID()) + " at state: " + std::to_string(currTransact->getCurrentState()) + "; model time: " \
                                + std::to_string(_modelTime) + ": advanced";                 
        _simLogs->logMess_transactLog(message);
    }
}

void SimCPP::generate(long double birthDelayInterval) { 
    Transact* currTransact = *_CECIt;
    std::string message;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    Transact* newTransact = new Transact(_maxId++,_modelTime + birthDelayInterval, 0, currTransact->getCurrentState());
    _FEC.emplace(std::find_if(_FEC.begin(),_FEC.end(),[ newTransact ](Transact* transact) {return newTransact->getTime() < transact->getTime();}), newTransact);

    //making logs
    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string(currTransact->getID()) + " at state: " + std::to_string((currTransact)->getCurrentState()) + "; model time: " \
                            + std::to_string(_modelTime) + ": generated a Xact:" + std::to_string((currTransact)->getID()) + " with birth time " \
                            + std::to_string((currTransact)->getTime()) + " at birth state " + std::to_string((currTransact)->getCurrentState()); 
        _simLogs->logMess_transactLog(message);
    }
    if (_simLogs->isEnable_CFECLog()) {
        message = "\"generation\" Xact:" + std::to_string(newTransact->getID()) + " model time: " + std::to_string(_modelTime) \
                                 + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n'; 
        _simLogs->logMess_CFECLog(message);
    }
};

void SimCPP::initGenerate(unsigned int birthState, long double birthTime) {
    std::string message;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    Transact* newTransact = new Transact(_maxId++,birthTime, 0, birthState);
    _FEC.emplace(std::find_if(_FEC.begin(),_FEC.end(),[ newTransact ](Transact* transact) {return newTransact->getTime() < transact->getTime();}), newTransact);

    //making logs
    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string(newTransact->getID()) + " generating an initializing transact with birth time " \
                            + std::to_string(newTransact->getTime()) + " at birth state " + std::to_string(newTransact->getNextState());                 
        _simLogs->logMess_transactLog(message);
    }
    
    if (_simLogs->isEnable_CFECLog()) {
        message = "\"init generation\" Xact:" + std::to_string(newTransact->getID()) + " model time: " + std::to_string(_modelTime) \
                                     + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n'; 
        _simLogs->logMess_CFECLog(message);
    }
};

unsigned int SimCPP::sysEvent() {
    std::string message;
    Transact* replTransact;

    //moving transactions from FEC to CEC if _CECIt at end
    while (_CECIt == _CEC.end()) {
        replTransact = *(_FEC.begin());
        _modelTime = replTransact->getTime();
        auto aaIt = std::find_if(_CEC.begin(), _CEC.end(), [ replTransact ](Transact* transact) {return replTransact->getTime() < transact->getTime();});
        _CEC.emplace(aaIt, replTransact);
        //_CEC.emplace(replTransact, [ replTransact ](Transact* transact) {return replTransact->getTime() < transact->getTime();});
        
        _FEC.erase(_FEC.begin());
        _CECIt = _CEC.begin();

        if (_simLogs->isEnable_CFECLog()) {
            message = "\"promotion of model time\" Xact:" + std::to_string(replTransact->getID()) + " model time: " + std::to_string(_modelTime)\
                             + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n'; 
            _simLogs->logMess_CFECLog(message);
        }
    }

    (*_CECIt)->setTime(_modelTime);

    (*_CECIt)->setCurrentState((*_CECIt)->getNextState());
    return (*_CECIt)->getCurrentState();
};

void SimCPP::terminate(unsigned int reduceCounter) {
    Transact* termTrans = *_CECIt;
    unsigned int termTransID = termTrans->getID();
    unsigned int termTransCurrState = termTrans->getCurrentState();
    std::string message;
    EventChain::iterator futIt = _CECIt;
    futIt++;

    if (reduceCounter >= this->_counter) {
        _counter = 0;

        if (_simLogs->isEnable_StatLog()) {
            message = _queues.getFinalStatString(_modelTime);
            message += '\n' + _storages.getFinalStatString(_modelTime);
            _simLogs->logMess_statLog(message);
        }
        _simLogs->modelEndMess("Simulation is ended!");
        delete _simLogs;
        _simLogs = nullptr;        
    }
    else {
        _counter -= reduceCounter;
        _CEC.erase(_CECIt);

        if (_CEC.needReset()) {
            futIt = _CEC.begin();
            _CEC.setReset(false);
        }

        _CECIt = futIt;

        if (_simLogs->isEnable_CFECLog()) {
            message = "\"terminating\" Xact:" + std::to_string(termTransID) + " model time: " + std::to_string(_modelTime) \
                        + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n'; 
            _simLogs->logMess_CFECLog(message);
        }

        if (_simLogs->isEnable_transactLog()) {
            message = "Xact:" + std::to_string(termTransID) + " at state: " + std::to_string(termTransCurrState) + "; model time: " \
                                + std::to_string(_modelTime) + ": terminated";                 
            _simLogs->logMess_transactLog(message);
        }
    }
        
};

long double SimCPP::getModelTime() { 
    return _modelTime; 
};

bool SimCPP::isRunning() {
    return this->_counter != 0;
};

void SimCPP::start(unsigned int count, std::ofstream* sysEvLog, std::ofstream* statLog,  std::ofstream* transactLog, std::ofstream* CFECLog) {
    if (this->isRunning())
        throw std::logic_error("You cannot start the model until it completes");
    _simLogs = new SimLogs(sysEvLog, statLog, transactLog, CFECLog);
    _simLogs->modelInitMess(_modelName);
    
    this->_counter = count;
    this->_maxId = 1;
}

unsigned int SimCPP::enter(const std::string storageName, const unsigned int numbOfChannels) {
    unsigned int seizedChannels;
    Transact* currTransact;
    std::string message;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    seizedChannels = _storages.enter(currTransact, storageName, numbOfChannels);

    if (_simLogs->isEnable_CFECLog()) {
        message = "\"seizing\" Xact:" + std::to_string((currTransact)->getID()) + " model time: " + std::to_string(_modelTime) \
                   + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n'; 
                _simLogs->logMess_CFECLog(message);
    }

    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string((currTransact)->getID()) + " at state: " + std::to_string((currTransact)->getCurrentState()) + "; model time: " \
                                + std::to_string(_modelTime) + ": seized " + std::to_string(seizedChannels) + " channel(s) at \"" + storageName + "\" storage";                 
        _simLogs->logMess_transactLog(message);
    }

    return seizedChannels;
}

void SimCPP::leave(const std::string storageName, const unsigned int numbOfChannels) {
    unsigned int releasedChannels;
    Transact* currTransact;
    std::string message;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;

    releasedChannels = _storages.leave(currTransact, storageName, numbOfChannels);
    _CEC.setReset(true); //will used while TERMINATE transact

    if (_simLogs->isEnable_CFECLog()) {
        message = "\"releazing\" Xact:" + std::to_string((currTransact)->getID()) + " model time: " + std::to_string(_modelTime) \
                   + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n'; 
                _simLogs->logMess_CFECLog(message);
    }

    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string((currTransact)->getID()) + " at state: " + std::to_string((currTransact)->getCurrentState()) + "; model time: " \
                                + std::to_string(_modelTime) + ": released " + std::to_string(releasedChannels) + " channel(s) at \"" + storageName + "\" storage";                 
        _simLogs->logMess_transactLog(message);
    }
}

void SimCPP::storage(const std::string storageName, const unsigned int _maxChannels) {
    if (this->isRunning()) {
        throw std::logic_error("You cannot interact with the model storages after \"start\"ing the model");
    }
    _storages.storageAppend(storageName, _maxChannels);
}

double SimCPP::exponential(double mean) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> dist(1. / mean);
    double randomValue = dist(gen);
    return randomValue;
}