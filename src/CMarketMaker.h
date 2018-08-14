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
    string productType;
    string productPropsFile;

    unordered_map<int,Product*> productMap;
    list<MarketInfo> mktdataList;
    
    virtual BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps);
    virtual initProducts(Product *productPtr);

    virtual int runStep();

    int processOrder(Order *order);
    int deleteProducts();

};

#endif