#pragma once

#include <C:\Users\666an\Documents\BMSTU\3SEM\Pract_C++\Practica_3\SimCPP\Transact.h>
#include <C:\Users\666an\Documents\BMSTU\3SEM\Pract_C++\Practica_3\SimCPP\EventChain.h>
#include <algorithm>
#include <string>
#include <vector>

class Links {
    friend class SimCPP;
    private:
        class Link;
        std::vector<Link*> _links;
        Links(){};
    public:
        std::string getAsString();
        void link(Transact* transact, const std::string linkName, const std::string discipline);
        std::vector<Transact*> unlink(const std::string linkName, const unsigned int numbReleasedTrans);
        unsigned int getLinkParam(const std::string linkName, const std::string SNA);
};

class Links::Link {
    private:
        EventChain _link;
    public:
        Link (const std::string name): _link(EventChain (name)) {}

        std::string getName() { return _link.getName(); }
        std::string getAsString() { return _link.getAsString(); }

        void link(Transact* transact, const std::string discipline);
        std::vector<Transact*> unlink(unsigned int numbReleasedTrans);
        unsigned int getLinkParam(const std::string SNA);
};

//-----

unsigned int Links::getLinkParam(const std::string linkName, const std::string SNA) {
    std::vector<Link*>::iterator linkIt = std::find_if(_links.begin(),_links.end(), [ linkName ](Links::Link* link){ return linkName == link->getName(); });
    if (linkIt == _links.end()) {
        return 0;
    }
    return (*linkIt)->getLinkParam(SNA);
}

void Links::link(Transact* insertedTransact, const::std::string linkName, const std::string discipline) {
    std::vector<Link*>::iterator linkIt = std::find_if(_links.begin(),_links.end(), [ linkName ](Links::Link* link){ return linkName == link->getName(); });
    if (linkIt == _links.end()) {
        _links.push_back(new Link(linkName));
        linkIt = _links.end();
        linkIt--;
    }
    (*linkIt)->link(insertedTransact,discipline);
}

std::vector<Transact*> Links::unlink(const std::string linkName, const unsigned int numbReleasedTrans) {
    std::vector<Links::Link*>::iterator linkIt = std::find_if(_links.begin(),_links.end(), [ linkName ](Links::Link* LINK){ return linkName == LINK->getName(); });
    if (linkIt == _links.end()) {
        _links.push_back(new Link(linkName)); //gpss style solution, ?throw as alter?
        linkIt = _links.end();
        linkIt--;
    }
    return (*linkIt)->unlink(numbReleasedTrans);
}

std::string Links::getAsString() {
    std::string message {"LINKS: "};
    std::for_each(_links.begin(),_links.end(),[ &message ](Links::Link* LINK){message += LINK->getAsString();});
    return message;
}

//-----

unsigned int Links::Link::getLinkParam(const std::string SNA) {
    if (SNA == "CH") {
        return _link.size();
    }
    throw std::logic_error("Unknown system numeric attribute \"" + SNA + '\"');
}

void Links::Link::link(Transact* insertedTransact, const std::string discipline) {
    //M1,FIFO,LIFO,PR
    if (discipline == "LIFO") {
        _link.emplace(_link.begin(), insertedTransact);
    }
    else if (discipline == "FIFO") {
        _link.emplace(_link.end(), insertedTransact);
    }
    else {
        _link.emplace(std::find_if(_link.begin(),_link.end(),[discipline,insertedTransact] (Transact* transact) \
            { return insertedTransact->getParam(discipline) < transact->getParam(discipline); }),insertedTransact);
    }
}

std::vector<Transact*> Links::Link::unlink(unsigned int numbReleasedTrans) {
    std::vector<Transact*> releasedTrans;
    EventChain::iterator linkIt = _link.begin();
    while (linkIt != _link.end() && numbReleasedTrans > 0) {
        releasedTrans.push_back(*linkIt);
        _link.erase(linkIt++);
        numbReleasedTrans--;
    }
    return releasedTrans;
}