
#ifndef OBJECTCLASS
#define OBJECTCLASS

#include "CommonTypes.h"

class MarketMaker;

using namespace std;

class Product
{
private:
    virtual int putTradeResult(repast::AgentId id, TradeResult trade) {};
    


public:
//Properties
    int productID;
    string productName;
    string productType;
    MarketMaker* market;

//Actions
    Product(MarketMaker* mkt):market(mkt){};
    virtual int processOrder(Order order) = 0;
    virtual MarketInfo *getMarketData() = 0;
    virtual TradeResult *getTradeResult() = 0;
    virtual int contractChecker() = 0;
    virtual Product* clone(MarketMaker *mkt, string strProps) = 0;
};

class Stock : public Product
{
private:
    OrderBook ordBook;
    MarketInfo mktInfo;
    list<int> groups;
    list<TradeResult> trdResultList;
 
    Stock(MarketMaker *mkt, string stockPropsString);
    ~Stock();
 
     Product* clone(MarketMaker *mkt, string productPropsString);

public:
    int processOrder(Order order);
    int putTradeResult(repast::AgentId id, TradeResult trade);
    TradeResult *getTradeResult();
    int contractChecker();
    MarketInfo *getMarketData();
};

#endif