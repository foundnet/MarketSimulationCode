#ifndef MARKETPART
#define MARKETPART

#include "ObjectClass.h"
#include "CAgent.h"

class MarketMaker;

using namespace std;

typedef struct _Condition
{
    bool curStatus;
    bool (*conFunctionPtr) (map<string, map<double, string>> *localParam, map<string, map<double, string>> *globalPram);
}Condition ;

typedef struct _ConStatus
{
    string conName;
    bool expStatus;
}ConStatus;

typedef struct _Rule
{
    list<ConStatus> conditionSet;
    int (*action) (map<string, map<double, string>> *localParam, map<string, map<double, string>> *globalPram);
}Rule ;


class MktParticipant : public BaseAgent {
public:
//Properties
    int category;                   
    int currency;                   //Cash   
    int currentAssets;              //Current assets

    vector<MarketMaker*> markets;
    unordered_map<int,MarketInfo> mktdataMap;
    unordered_map<int,Holding>  holdingMap;            //The holding info

    map<string, Condition> conditionMap;          //Condition List 
    list<Rule> ruleTable;                   //Rule table
    list<Order> orderList;

    vector<TradeResult>  pastTrades;

//Actions
    MktParticipant() {};
    MktParticipant(repast::AgentId id, repast::Properties *agentProps);
    ~MktParticipant();
    BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps);

    int messageProcessor(MessageInfo *info);
    int handleStepWork();

//Decision Function
    virtual int conditionCheck();
    virtual int actionExecuter();
    virtual int expectationGenerator();

//Communication Function
    void registerMarkets(vector<MarketMaker*> markets);
    int sendOrders(Order *order);

};

#endif