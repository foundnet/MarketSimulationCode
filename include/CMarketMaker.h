#ifndef MARKETMAKER
#define MARKETMAKER

#include "ObjectClass.h"
#include "CAgent.h"


class MarketMaker : public BaseAgent {
public:
    MarketMaker();
    MarketMaker(repast::AgentId id, repast::Properties *agentProps);
    ~MarketMaker();

    int productCount;
    string marketName;
    string productType;
    string productPropsFile;

    unordered_map<int,Product*> productMap;
    unordered_map<int,MarketInfo> mktdataMap;
    
    BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps);
    int messageProcessor(MessageInfo *info);
    int handleStepWork();

    int initProducts(Product *productPtr);

    int processOrder(Order *order);
    void deleteProducts();
    int bcastMarketInfo(MarketInfo *mktInfo);

};

#endif
