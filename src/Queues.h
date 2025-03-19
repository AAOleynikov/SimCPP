#pragma once

#include "Transact.h"
#include <string>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <tuple>
#include <list>

class Queues {
    friend class SimCPP;
    private:
        class Queue;
        std::vector<Queue*> _queues;
        
        Queues(){}
        std::vector<Queue*>::iterator chooseQueue(const std::string queueName);
    public:
        void queue(const std::string queueName, Transact* transact);
        void depart(const std::string queueName, Transact* transact);
        std::string getFinalStatString(long double endModelTime);
};

class Queues::Queue {
    private:
        const std::string _queueName;
        std::list<std::tuple<Transact*,long double>> _queue;
        unsigned long _numbRegTrans; //number of reg. trans. at queue
        unsigned long _nullnumbRegTrans; //avTime(-0) in queue = cumSumTime / numbRegTrans(-0) (if time in _queue not 0)
        long double _cumSumTime; //avTime in queue = cumSumTime / numbRegTrans
        long double _cumSumCont; //AVE.CONT. = _cumSumCont / endModelTime
        long double _prevQueueTime; //_cumSumCont = (currTransTime - prevQueueTime) * _currQueueLength
        unsigned long _maxQueueLength;
        unsigned long _currQueueLength;

        void departStat(long double prevTransTime, long double currTransTime);
        void queueStat(long double currTransTime);
    public:
        Queue (const std::string queueName): _queueName(queueName), _numbRegTrans(0), _nullnumbRegTrans(0), _cumSumTime(.0), \
            _maxQueueLength(0), _currQueueLength(0), _cumSumCont(0), _prevQueueTime(0) {}

        std::string getName() { return _queueName; }
        void queue(Transact* transact);
        void depart(Transact* transact);
        std::string getFinalStatString(long double endModelTime);
        static std::string getFinalStatMeaningString() {return "QUEUE\t\tMAX\tCONT.\tENTRY\tENTRY(0)\tAVE.CONT.\tAVE.TIME\tAVE.(-0)"; }
};

//-----

std::string Queues::getFinalStatString(long double endModelTime) {
    std::string message = '\n' + Queue::getFinalStatMeaningString();
    std::for_each(_queues.begin(),_queues.end(),[&message, endModelTime](Queues::Queue* queue) \
        { message += '\n' + queue->getFinalStatString(endModelTime); });
    return message;
}

std::vector<Queues::Queue*>::iterator Queues::chooseQueue(const std::string queueName) {
    std::vector<Queue*>::iterator queuesIt = std::find_if(_queues.begin(), _queues.end(), \
        [ queueName ](Queues::Queue* queue){return queueName == queue->getName();});
    return queuesIt;
}

void Queues::queue(const std::string queueName, Transact* transact) {
    std::vector<Queue*>::iterator queuesIt = this->chooseQueue(queueName);
    if (queuesIt == _queues.end()) {
        _queues.push_back(new Queue(queueName));
        queuesIt = _queues.end(); queuesIt--;
    }
    (*queuesIt)->queue(transact);
}

void Queues::depart(const std::string queueName, Transact* transact) {
    std::vector<Queue*>::iterator queuesIt = this->chooseQueue(queueName);
    if (queuesIt == _queues.end()) {
        throw std::logic_error("Illegal attempt to make Queue entity content negative at \"" + queueName + "\" queue");
    }
    (*queuesIt)->depart(transact);
}

//-----

void Queues::Queue::queue(Transact* transact) {
    _queue.push_back({transact,transact->getTime()});
    this->queueStat(transact->getTime());
}

void Queues::Queue::depart(Transact* transact) {
    std::tuple<Transact*,long double> data;
    long double currTransTime = transact->getTime();

    std::list<std::tuple<Transact*,long double>>::iterator QIt = std::find_if(_queue.begin(),_queue.end(), \
        [transact](std::tuple<Transact*,long double> data){return transact == std::get<0>(data);});

    if (QIt == _queue.end()) {
        throw std::logic_error("Illegal attempt to make Queue entity content negative at \"" + _queueName + "\" queue");
    }

    data = *QIt;
    this->departStat(std::get<1>(data), currTransTime);

    _queue.erase(QIt);
}

void Queues::Queue::departStat(long double prevTransTime, long double currTransTime) {
    _cumSumTime += currTransTime - prevTransTime;

    _cumSumCont += (currTransTime - _prevQueueTime) * _currQueueLength;
    _prevQueueTime = currTransTime;
    _currQueueLength--;

    if (currTransTime == prevTransTime) {
        _nullnumbRegTrans++;
    }
}

void Queues::Queue::queueStat(long double currTransTime) {
    _numbRegTrans++;

    _cumSumCont += (currTransTime - _prevQueueTime) * _currQueueLength;
    _prevQueueTime = currTransTime;
    _currQueueLength++;

    if (_currQueueLength > _maxQueueLength) {
        _maxQueueLength = _currQueueLength;
    } 
}

std::string Queues::Queue::getFinalStatString(long double endModelTime) {

    std::string statString = _queueName + '\t';
    std::string avTimeStr, noNullAvTimeStr, avContStr;
    unsigned long contAtQueue = 0; //CONT.

    std::for_each(_queue.begin(),_queue.end(),[ endModelTime,this,&contAtQueue ](std::tuple<Transact*,long double> data) \
        { this->departStat(std::get<1>(data), endModelTime); contAtQueue++; });
    _queue.clear();

    if (_numbRegTrans != 0) {
        avTimeStr = std::to_string(_cumSumTime / _numbRegTrans);
    }
    else {
        avTimeStr = "------";
    }

    if (_numbRegTrans - _nullnumbRegTrans != 0) {
        noNullAvTimeStr = std::to_string(_cumSumTime / (_numbRegTrans - _nullnumbRegTrans));
    }
    else {
        noNullAvTimeStr = "------";
    }

    if (endModelTime > 0) {
        avContStr = std::to_string(_cumSumCont / endModelTime);
    }
    else {
        avContStr = "------";
    }

    statString += std::to_string(_maxQueueLength) + '\t' + std::to_string(contAtQueue) + '\t' + std::to_string(_numbRegTrans) + "\t" \
        + std::to_string(_nullnumbRegTrans) + "\t\t" + avContStr + '\t' + avTimeStr + '\t' + noNullAvTimeStr;
    
    return statString;
}