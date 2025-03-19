#pragma once

#include "Transact.h"
#include <algorithm>
#include <string>
#include <list>

class EventChain {
    friend class SimCPP;
    friend class Links;

    private:
        std::list<Transact*> _evChain;
        const std::string _name;
        bool _resetEvChain;

        EventChain(const std::string name): _name(name) {_resetEvChain = false;}
    public:
        ~EventChain() {std::for_each(_evChain.begin(),_evChain.end(),[](Transact* transact){delete transact;});}

        using iterator = std::list<Transact*>::iterator;
        using const_iterator = std::list<Transact*>::const_iterator;
        
        iterator begin() { return _evChain.begin(); }
        iterator end() { return _evChain.end(); }
        const_iterator cbegin() const { return _evChain.cbegin(); }
        const_iterator cend() const { return _evChain.cend(); }

        const std::string getName() { return _name; }
        unsigned int size() { return _evChain.size(); }
        bool needReset() { return _resetEvChain; }
        void setReset(bool reset = true) { _resetEvChain = reset; }
        EventChain::iterator emplace(EventChain::iterator evChainIt, Transact* transact) { return _evChain.emplace(evChainIt, transact); }
        void eraseTrans(Transact* transact) { _evChain.erase(std::find(_evChain.begin(),_evChain.end(),transact)); }
        void erase(EventChain::iterator evChainIt) { _evChain.erase(evChainIt); }

        //void clear();
        EventChain::iterator skipBlocked(EventChain::iterator evChainIt); //if we cant move trans we skip it
        const std::string getAsString();
};

//-----

const std::string EventChain::getAsString() {
    std::string evS {_name + ":   "};
    std::list<Transact*>::iterator evIt = _evChain.begin();
    while(evIt != _evChain.end()) {
        evS += (*evIt)->getAsString() + ' ';
        evIt++;
    }
    return evS;
}

EventChain::iterator EventChain::skipBlocked(EventChain::iterator evChainIt) {
    while((evChainIt != this->end()) && (*evChainIt)->isBlocked()) {
        evChainIt++;
    }
    return evChainIt;
}