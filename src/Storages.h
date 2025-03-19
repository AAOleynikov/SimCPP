#pragma once

#include "Transact.h"
#include <algorithm>
#include <stdexcept>
#include <vector>

class Storages {
    friend class SimCPP;

    private:
        class Storage;
        std::vector<Storage*> _storages;

        Storages(){};
        std::vector<Storage*>::iterator chooseStorage(const std::string storageName);
        void storageAppend(const std::string storageName, const unsigned int maxChannels);
        bool contains(const std::string storageName);
    public:
        unsigned int enter(Transact* transact, const std::string storageName, const unsigned int numbOfChannels);
        unsigned int leave(Transact* transact, const std::string storageName, const unsigned int numbOfChannels);
        unsigned int getStorageParam(const std::string storageName, const std::string SNA);
        std::string getFinalStatString(long double endModelTime);
};

class Storages::Storage {
    private:
        const unsigned int _maxChannels;
        unsigned int _currChannels; //count of seized! channels
        const std::string _storageName;

        unsigned long _numbEnterTrans; //number of entered. trans. at storage
        unsigned long _maxProcessLength; //MAX.
        long double _cumSumCont; //AVE.CONT. = _cumSumCont / endModelTime
        long double _prevStorageTime; //_cumSumCont = (currTransTime - prevStorageTime) * currChannels

        void enterStat(unsigned long numbOfChannels, long double currTransTime);
        void leaveStat(unsigned long numbOfChannels, long double currTransTime);
    public:
        Storage(const std::string name, const unsigned int maxChannels): _storageName(name), _maxChannels(maxChannels), _currChannels(0), \
            _numbEnterTrans(0), _maxProcessLength(0), _cumSumCont(.0), _prevStorageTime(0) {};
        unsigned int enter(Transact* transact, const unsigned int numbOfChannels);
        unsigned int leave(Transact* transact, const unsigned int numbOfChannels);
        unsigned int getStorageParam(const std::string storageName, const std::string SNA);
        const std::string getName() { return _storageName; }
        static std::string getFinalStatMeaningString() { return "STORAGE\t\tCAP.\tMIN.\tMAX.\tENTRIES\t\tAVE.C.\t\tUTIL."; }
        std::string getFinalStatString(long double endModelTime);
};

//-----

std::string Storages::getFinalStatString(long double endModelTime) {
    std::string message = '\n' + Storages::Storage::getFinalStatMeaningString();
    std::for_each(_storages.begin(),_storages.end(),[&message, endModelTime](Storages::Storage* storage) \
        { message += '\n' + storage->getFinalStatString(endModelTime); });
    return message;
}

std::vector<Storages::Storage*>::iterator Storages::chooseStorage(const std::string storageName) {
    std::vector<Storage*>::iterator storagesIt = std::find_if(_storages.begin(), _storages.end(), \
        [ storageName ](Storages::Storage* storage){return storageName == storage->getName();});
    if (storagesIt == _storages.end())
        throw std::logic_error("You cannot enter/leave unannounced storage ()" + storageName + ')');
    return storagesIt;
}

unsigned int Storages::enter(Transact* transact, const std::string storageName, const unsigned int numbOfChannels) {
    unsigned int seizedChannels = (*chooseStorage(storageName))->enter(transact, numbOfChannels);
    return seizedChannels;
}
        
unsigned int Storages::leave(Transact* transact, const std::string storageName, const unsigned int numbOfChannels) {
    unsigned int releasedChannels = (*chooseStorage(storageName))->leave(transact, numbOfChannels);
    return releasedChannels;
}

bool Storages::contains(const std::string storageName) {
    return (std::find_if(_storages.begin(),_storages.end(),[ storageName ]( Storages::Storage* storage) \
            {return storageName == storage->getName();}) != _storages.end());
}

void Storages::storageAppend(const std::string storageName, const unsigned int maxChannels) {
    if (this->contains(storageName))
        throw std::logic_error("You cannot create storages with the same names (" + storageName + ')');
    
    _storages.emplace_back(new Storages::Storage (storageName, maxChannels));
}

//-----

void Storages::Storage::enterStat(unsigned long numbOfChannels, long double currTransTime) {
    _numbEnterTrans++;
    _cumSumCont += (currTransTime - _prevStorageTime)*(_currChannels - numbOfChannels);
    _prevStorageTime = currTransTime;
    if (_currChannels > _maxProcessLength) {
        _maxProcessLength = _currChannels;
    }
}

void Storages::Storage::leaveStat(unsigned long numbOfChannels, long double currTransTime) {
    _cumSumCont += (currTransTime - _prevStorageTime)*(_currChannels + numbOfChannels);
    _prevStorageTime = currTransTime;
}

std::string Storages::Storage::getFinalStatString(long double endModelTime) {
    std::string statString = _storageName + '\t';
    std::string avCountStr, UTILStr; //AVE.C. UTIL.
    this->leaveStat(0, endModelTime);

    if (endModelTime > 0) {
        avCountStr = std::to_string(_cumSumCont / endModelTime);
        UTILStr = std::to_string(_cumSumCont / endModelTime / _maxChannels);
    }
    else {
        avCountStr = "------";
        UTILStr = "------";
    }

    statString += std::to_string(_maxChannels) + '\t' + std::to_string(_maxChannels - _currChannels) + '\t' + \
        std::to_string(_maxProcessLength) + "\t" + std::to_string(_numbEnterTrans) + "\t\t" + avCountStr + '\t' + UTILStr;
    
    return statString;
}

unsigned int Storages::Storage::enter(Transact* transact, const unsigned int numbOfChannels) {
    if (numbOfChannels > _maxChannels)
        throw std::logic_error("Storage request exceeds total capacity (" + _storageName + ')');

    if (numbOfChannels > (_maxChannels - _currChannels)) {
        return 0;
    }
    else {
        _currChannels += numbOfChannels;
    }
    this->enterStat(numbOfChannels, transact->getTime());
    return numbOfChannels;
}

unsigned int Storages::Storage::leave(Transact* transact, const unsigned int numbOfChannels) {
    if (numbOfChannels > _currChannels) {
            throw std::logic_error("Attempt to release more storage than existed (" + _storageName + ')');
    }

    _currChannels -= numbOfChannels;

    this->leaveStat(numbOfChannels, transact->getTime());
    return numbOfChannels;
}

unsigned int Storages::getStorageParam(const std::string storageName, const std::string SNA) {
    return (*chooseStorage(storageName))->getStorageParam(storageName, SNA);
}

unsigned int Storages::Storage::getStorageParam(const std::string storageName, const std::string SNA) {
    if (SNA == "CH") {
        return (_currChannels);
    }
    else if (SNA == "R")
    {
        return (_maxChannels - _currChannels);
    }
    throw std::logic_error("Unknown system numeric attribute \"" + SNA + '\"');
}