#ifndef MARKETPART
#define MARKETPART

#include "ObjectClass.h"
#include "CAgent.h"

class MarketMaker;

using namespace std;

class MktParticipant : public BaseAgent {
public:
//Properties
    int category;                   
    int currency;                   //Cash   
    
    int currentAssets;              //Current assets

    vector<MarketMaker*> markets;
    unordered_map<int,MarketInfo> mktdataMap;
    unordered_map<int,Holding>  holdingMap;            //The holding info

    vector<Trade>  pastTrades;

//Actions
    MktParticipant() {};
    ~MktParticipant();
    MktParticipant(repast::AgentId id, repast::Properties *agentProps);

    BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps);
    int handleInformation(Information *info);
    int handleStepWork();

    void registerMarkets(vector<MarketMaker*> markets);
    int sendOrders(Order *order);
    int calculateExpectation(int productID);

};

#endif