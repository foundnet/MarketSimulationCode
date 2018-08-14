#ifndef MARKETMAKER
#define MARKETMAKER

#include "CommonTypes.h"
#include "CAgent.h"

class MarketMaker : public BaseAgent {
public:
    MarketMaker(repast::AgentId id, repast::Properties* agentProps);
    ~MarketMaker();

    int productCount;
    string marketName;

    unordered_map<Product*> products;
    list<MarketInfo> mktdataList;
    
    virtual BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps);
    virtual int runStep();

    int processOrder(Order *order);
};

#endif