#include "CommonTypes.h"
#include "CAgent.h"

class MarketMaker : public BaseMktAgent {
public:
    MarketMaker(string cfgfile, int stockCount, int investorCount);
    ~MarketMaker();
    int InputOrders(Order *pOrder,int cnt);
    int productCount;
    
    MarketInfo market;
    OrderBook *orderbook;
    
    
private:
    int exchangeStatus=0;       //0-Before Start  1-Trading  2-Post Trade

};