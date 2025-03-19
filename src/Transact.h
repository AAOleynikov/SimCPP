#pragma once

#include <map>
#include <string>
#include <stdexcept>

class Transact {
    friend class SimCPP;

    private:
        unsigned long _ID;
        long double _timeNextEvent;
        unsigned int _currentState;
        unsigned int _nextState;

        Transact(unsigned int ID, float timeNextEvent, unsigned int currentState, unsigned int nextState): 
            _ID(ID), _timeNextEvent(timeNextEvent), _currentState(currentState), _nextState(nextState) {}
    public:
        unsigned long getID() { return _ID; }
        unsigned int getCurrentState() { return _currentState; }
        void setCurrentState(unsigned int state) { _currentState = state; }
        unsigned int getNextState() { return _nextState; }
        void setNextState(unsigned int state) { _nextState = state; }
        long double getTime() { return _timeNextEvent; }
        void setTime(long double time) { _timeNextEvent = time; }
        static std::string getTransactMeaningString() { return "{ID; time next event; current state; next state}"; }

        std::string getAsString();  
};

//-----

std::string Transact::getAsString() {
    std::string TrStr {'{' + std::to_string(_ID) + "; " + std::to_string(_timeNextEvent) \
                        + "; " + std::to_string(_currentState) + "; " + std::to_string(_nextState) + '}'}; 
    return TrStr;
}