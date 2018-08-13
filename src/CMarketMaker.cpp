#include "CMarketMaker.h"

MarketMaker::MarketMaker(repast::AgentId id, repast::Properties* agentProps):BaseAgent(id)
{
 	int stopAt = repast::strToInt(agentProps->getProperty("stop.at"));
   stockCnt = stockCount;

    ifstream file(cfgfile,ios::in|ios::binary);  
    for (int i=0; i<stockCnt;i++) {
        file.read((char *)&market[i], sizeof(MarketInfo));
    }
    file.close();
}

MarketMaker::~MarketMaker() {
    delete market;
}

int MarketMaker::InputOrders(Order *pOrder, int cnt) {
    if (exchangeStatus == 0) {
        exchangeStatus = 1;
        return 1;
    }
    else return 0;
}

BaseAgent* MarketMaker::clone(repast::AgentId id, repast::Properties* agentProps)
{
    MarketMaker *maker = new MarketMaker(id, agentProps);
    return &maker;

}