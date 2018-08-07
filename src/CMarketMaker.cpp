#include "CMarketMaker.h"

MarketMaker::MarketMaker(string cfgfile, int stockCount, int investorCount) {
 
    stockCnt = stockCount;
    investorCnt = investorCount;
    market = new MarketInfo[stockCnt];
    orderbook = new OrderBook[stockCnt];

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
