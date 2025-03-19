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
        bool _blocked; //it can be locked in the seize and enter blocks
        std::map<const std::string,long double> _params; //string value/int value at c++ 17 "std::variant"

        Transact(unsigned int ID, float timeNextEvent, unsigned int currentState, unsigned int nextState): 
            _ID(ID), _timeNextEvent(timeNextEvent), _currentState(currentState), _nextState(nextState) 
            {_blocked = false; _params["M1"] = _timeNextEvent;}
    public:
        unsigned long getID() { return _ID; }
        unsigned int getCurrentState() { return _currentState; }
        void setCurrentState(unsigned int state) { _currentState = state; }
        unsigned int getNextState() { return _nextState; }
        void setNextState(unsigned int state) { _nextState = state; }
        long double getTime() { return _timeNextEvent; }
        void setTime(long double time) { _timeNextEvent = time; }
        void block() { _blocked = true; }
        void unBlock() { _blocked = false; }
        bool isBlocked() { return _blocked; }
        void setParam(const std::string paramName, long double value) { _params[paramName] = value; }
        static std::string getTransactMeaningString() { return "{ID; time next event; current state; next state; is blocked}"; }

        long double getParam(const std::string paramName);
        std::string getAsString();  
};

//-----

long double Transact::getParam(const std::string paramName) {
    std::map<const std::string,long double>::iterator paramIter = _params.find(paramName); //at c++ 20 "contains"
    if (paramIter == _params.end()) {
        throw std::logic_error("Reference to a non-existent Parameter(" + paramName + ") at state:" + std::to_string(_currentState));
    }
    return _params[paramName];
}

std::string Transact::getAsString() {
    std::string TrStr {'{' + std::to_string(_ID) + "; " + std::to_string(_timeNextEvent) \
                        + "; " + std::to_string(_currentState) + "; " + std::to_string(_nextState) + "; " + std::to_string(_blocked) + '}'}; 
    return TrStr;
}