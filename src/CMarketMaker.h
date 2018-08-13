#ifndef MARKETMAKER
#define MARKETMAKER

#include "CommonTypes.h"
#include "CAgent.h"

class MarketMaker : public BaseAgent {
public:
    MarketMaker(string cfgfile, int productCount);
    virtual ~MarketMaker();

    int productCount;

    list<Product> productList;
    list<MarketInfo> mktdataList;
    
    virtual BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps);
    int processOrder(Order *order);
};

#endif