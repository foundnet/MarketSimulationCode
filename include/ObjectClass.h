
#ifndef OBJECTCLASS
#define OBJECTCLASS

#include "CommonTypes.h"

class MarketMaker;

using namespace std;

class Product
{
public:
//Properties
    int productID;
    string productName;
    string productType;
    MarketMaker* market;
//Actions
    Product(MarketMaker* mkt):market(mkt){};
    virtual int matchOrder(Order order) = 0;
    virtual int sendTrades(repast::AgentId id, Trade trade) {};
    virtual int sendOrderConfirm(repast::AgentId id, OrderConfirm ordCnfm){};    
    virtual MarketInfo *getMarketData(){};
    virtual Product* clone(MarketMaker *mkt, string strProps) = 0;
};

class Stock : public Product
{
private:
    OrderBook ordBook;
    MarketInfo mktInfo;
    list<int> groups;

    Stock(MarketMaker *mkt, string stockPropsString);
    ~Stock();
    Product* clone(MarketMaker *mkt, string productPropsString);

public:
    int matchOrder(Order order);
    int sendTrades(repast::AgentId id, Trade trade);
    int sendOrderConfirm(repast::AgentId id, OrderConfirm ordCnfm);    
    MarketInfo *getMarketData();
    
};

#endif