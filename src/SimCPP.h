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
#include "Links.h"

class SimCPP {
    private:
        const std::string _modelName;
        unsigned long _maxId;   //current max ID of Transact
        long double _modelTime; //current model time
        unsigned int _counter;  //analog GPSS START directive argument (START _counter)

        EventChain _FEC; //feature event chain
        EventChain _CEC; //current event chain
        EventChain::iterator _CECIt;
        Links _links;
        SimLogs* _simLogs;
        Storages _storages;
        Queues _queues;

        //void SimCPPEnd();
    public:
        SimCPP (std::string modelName): _modelName(modelName), _maxId(1), _modelTime(.0), _counter(0), \
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
        void assign(const std::string paramName, const long double value);
        void test(const bool switchRoute, const unsigned int ifFalseState);

        void queue(const std::string queueName);
        void depart(const std::string queueName);

        void advance(long double delay);
        void enter(const std::string name, const unsigned int numbOfChannels = 1);
        void leave(const std::string name, const unsigned int numbOfChannels = 1);
        void transfer(const unsigned int nextState);
        void link(const std::string linkName, const std::string discipline);
        void unlink(const std::string linkName, const unsigned int nextState, const unsigned int numbReleasedTrans);
        unsigned int getStorageParam(const std::string storageName, const std::string SNA);
        unsigned int getLinkParam(const std::string linkName, const std::string SNA);
        double exponential(double mean);
};

//-----

unsigned int SimCPP::getLinkParam(const std::string linkName, const std::string SNA) {
        if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }
    return _links.getLinkParam(linkName, SNA);
}

void SimCPP::queue(const std::string queueName) {
    Transact* currTransact;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    _queues.queue(queueName, currTransact);
    (currTransact)->setNextState((currTransact)->getCurrentState()+1);
}

void SimCPP::depart(const std::string queueName) {
    Transact* currTransact;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    _queues.depart(queueName, currTransact);
    (currTransact)->setNextState((currTransact)->getCurrentState()+1);
}

void SimCPP::test(const bool switchRoute, const unsigned int ifFalseState) {
    Transact* currTransact = *_CECIt;
    std::string message;
    
    if (switchRoute)
        currTransact->setNextState(currTransact->getCurrentState()+1);
    else
        currTransact->setNextState(ifFalseState);

    if (_simLogs->isEnable_CFECLog()) {
        message = "\"test\" Xact:" + std::to_string(currTransact->getID()) + " model time: " + std::to_string(_modelTime) \
                                     + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
        _simLogs->logMess_CFECLog(message);
    }

    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string(currTransact->getID()) + " at state: " + std::to_string(currTransact->getCurrentState()) + "; model time: " \
                                + std::to_string(_modelTime) + ": tested and will transfered to state:" + std::to_string(currTransact->getNextState());                 
        _simLogs->logMess_transactLog(message);
    }
}

void SimCPP::assign(const std::string paramName, const long double value) {
    Transact* currTransact = *_CECIt;
    std::string message;

    currTransact->setParam(paramName, value);
    currTransact->setNextState(currTransact->getCurrentState()+1);

    if (_simLogs->isEnable_CFECLog()) {
        message = "\"assign parameter\" Xact:" + std::to_string(currTransact->getID()) + " model time: " + std::to_string(_modelTime) \
                                     + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
        _simLogs->logMess_CFECLog(message);
    }

    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string(currTransact->getID()) + " at state: " + std::to_string(currTransact->getCurrentState()) + "; model time: " \
                                + std::to_string(_modelTime) + ": assign parameter: \"" + paramName + "\" with value:" + std::to_string(value);                 
        _simLogs->logMess_transactLog(message);
    }
}

void SimCPP::transfer(const unsigned int nextState) {
    std::string message;
    Transact* currTransact = *_CECIt;
    currTransact->setNextState(nextState);

    if (_simLogs->isEnable_CFECLog()) {
        message = "\"transfer\" Xact:" + std::to_string(currTransact->getID()) + " model time: " + std::to_string(_modelTime) \
                                     + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
        _simLogs->logMess_CFECLog(message);
    }

    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string(currTransact->getID()) + " at state: " + std::to_string(currTransact->getCurrentState()) + "; model time: " \
                                + std::to_string(_modelTime) + ": transfered to pos:" + std::to_string(nextState);                 
        _simLogs->logMess_transactLog(message);
    }
}

void SimCPP::advance(long double delay) {
    Transact* currTransact;
    std::string message;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    _CECIt++;

    currTransact->setNextState(currTransact->getCurrentState()+1); 
    currTransact->setTime(_modelTime + delay);
    _CEC.eraseTrans(currTransact);
    _FEC.emplace(std::find_if(_FEC.begin(),_FEC.end(),[ currTransact ](Transact* transact) {return currTransact->getTime() < transact->getTime();}), currTransact);

    if (_simLogs->isEnable_CFECLog()) {
                message = "\"advance\" Xact:" + std::to_string(currTransact->getID()) + " model time: " + std::to_string(_modelTime) \
                                     + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
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

    (currTransact)->setNextState((currTransact)->getCurrentState()+1); 

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
                                 + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
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
                                     + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
        _simLogs->logMess_CFECLog(message);
    }
};

unsigned int SimCPP::sysEvent() {
    std::string message;
    Transact* replTransact;
    //if we cant move trans we skip it
    _CECIt = _CEC.skipBlocked(_CECIt);

    //moving transactions from FEC to CEC if _CECIt at end
    while (_CECIt == _CEC.end()) {
        replTransact = *(_FEC.begin());
        _modelTime = replTransact->getTime();
        auto aaIt = std::find_if(_CEC.begin(), _CEC.end(), [ replTransact ](Transact* transact) {return replTransact->getTime() < transact->getTime();});
        _CEC.emplace(aaIt, replTransact);
        //_CEC.emplace(replTransact, [ replTransact ](Transact* transact) {return replTransact->getTime() < transact->getTime();});
        
        _FEC.erase(_FEC.begin());
        _CECIt = _CEC.begin();
        _CECIt = _CEC.skipBlocked(_CECIt);

        if (_simLogs->isEnable_CFECLog()) {
            message = "\"promotion of model time\" Xact:" + std::to_string(replTransact->getID()) + " model time: " + std::to_string(_modelTime)\
                             + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
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
                        + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
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

void SimCPP::enter(const std::string storageName, const unsigned int numbOfChannels) {
    unsigned int seizedChannels;
    Transact* currTransact;
    std::string message;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    seizedChannels = _storages.enter(currTransact, storageName, numbOfChannels);
    if (numbOfChannels == seizedChannels) {
        (currTransact)->setNextState((currTransact)->getCurrentState()+1); 
    }

    if (_simLogs->isEnable_CFECLog()) {
        message = "\"seizing\" Xact:" + std::to_string((currTransact)->getID()) + " model time: " + std::to_string(_modelTime) \
                   + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
                _simLogs->logMess_CFECLog(message);
    }

    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string((currTransact)->getID()) + " at state: " + std::to_string((currTransact)->getCurrentState()) + "; model time: " \
                                + std::to_string(_modelTime) + ": seized " + std::to_string(seizedChannels) + " channel(s) at \"" + storageName + "\" storage";                 
        _simLogs->logMess_transactLog(message);
    }
}


void SimCPP::leave(const std::string storageName, const unsigned int numbOfChannels) {
    unsigned int releasedChannels;
    Transact* currTransact;
    std::string message;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    (currTransact)->setNextState((currTransact)->getCurrentState()+1);

    releasedChannels = _storages.leave(currTransact, storageName, numbOfChannels);
    _CEC.setReset(true); //will used while TERMINATE transact
     

    if (_simLogs->isEnable_CFECLog()) {
        message = "\"releazing\" Xact:" + std::to_string((currTransact)->getID()) + " model time: " + std::to_string(_modelTime) \
                   + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
                _simLogs->logMess_CFECLog(message);
    }

    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string((currTransact)->getID()) + " at state: " + std::to_string((currTransact)->getCurrentState()) + "; model time: " \
                                + std::to_string(_modelTime) + ": released " + std::to_string(releasedChannels) + " channel(s) at \"" + storageName + "\" storage";                 
        _simLogs->logMess_transactLog(message);
    }
}


void SimCPP::link(const std::string linkName, const std::string discipline) {
    std::string message;
    Transact* currTransact;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    currTransact->setNextState(currTransact->getCurrentState());

    _links.link(currTransact,linkName,discipline);
    _CEC.erase(_CECIt++);

    if (_simLogs->isEnable_CFECLog()) {
        message = "\"linking\" Xact:" + std::to_string((currTransact)->getID()) + " to \"" + linkName + "\" model time: " + std::to_string(_modelTime) \
                   + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
        _simLogs->logMess_CFECLog(message);
    }

    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + std::to_string((currTransact)->getID()) + " at state: " + std::to_string((currTransact)->getCurrentState()) + "; model time: " \
                                + std::to_string(_modelTime) + ": linking to \"" + linkName + "\" with \"" + discipline + "\" discipline";                 
        _simLogs->logMess_transactLog(message);
    }
}

void SimCPP::unlink(const std::string linkName, const unsigned int nextState, const unsigned int numbReleasedTrans) {
    std::string message;
    std::string transIDString;
    std::vector<Transact*> releasedTrans;
    std::list<Transact*>::iterator emplaceIt = _CECIt;
    Transact* currTransact;

    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }

    currTransact = *_CECIt;
    currTransact->setNextState(currTransact->getCurrentState()+1);

    releasedTrans = _links.unlink(linkName,numbReleasedTrans);

    //emplasing to _CEC each transact after currTransact, setting current model time and setting unlink state 
    std::for_each(releasedTrans.begin(), releasedTrans.end(), [ this,nextState,&emplaceIt ] (Transact* emplTransact) \
        { emplTransact->setTime(this->getModelTime()); emplTransact->setNextState(nextState); emplaceIt++;
            emplaceIt = (this->_CEC.emplace(emplaceIt, emplTransact));});

    //making transact ID string
    std::for_each(releasedTrans.begin(),releasedTrans.end(),[ &transIDString ](Transact* transact){ transIDString += std::to_string(transact->getID()) + ';';});   

    if (_simLogs->isEnable_CFECLog()) {
        message = "\"unlinking\" Xact:" + transIDString + " from \"" + linkName + "\" to state:" + std::to_string(nextState) + " model time: " + std::to_string(_modelTime) \
                   + '\n' + _FEC.getAsString() + '\n' + _CEC.getAsString() + '\n' + _links.getAsString() + '\n'; 
        _simLogs->logMess_CFECLog(message);
    }

    if (_simLogs->isEnable_transactLog()) {
        message = "Xact:" + transIDString + " at state: " + std::to_string((currTransact)->getCurrentState()) + "; model time: " \
                                + std::to_string(_modelTime) + ": unlinking from \"" + linkName;                 
        _simLogs->logMess_transactLog(message);
    }
}

void SimCPP::storage(const std::string storageName, const unsigned int _maxChannels) {
    if (this->isRunning()) {
        throw std::logic_error("You cannot interact with the model storages after \"start\"ing the model");
    }
    _storages.storageAppend(storageName, _maxChannels);
}

unsigned int SimCPP::getStorageParam(const std::string storageName, const std::string SNA) {
    if (!this->isRunning()) {
        throw std::logic_error("You cannot interact with the model until you initialize it with \"start\"");
    }
    return _storages.getStorageParam(storageName, SNA);
}

double SimCPP::exponential(double mean) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> dist(1. / mean);
    double randomValue = dist(gen);
    return randomValue;
}