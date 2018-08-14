#include <iostream>
#include <string>
#include "CommonTypes.h"
#include "CMarketMaker.h"

using namespace std;


struct BaseObject
{

};

struct _MessageHead
{
    repast::AgentId receiverID;
    repast::AgentId senderID;
    int msgType;
    int bodyLength;
} ;

typedef struct _Information : BaseObject
{
    _MessageHead msgHead;
    unsigned char body[MAX_MSG_LEN - sizeof(_MessageHead)];
} Information;

class Product : BaseObject
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
    virtual int sendTrades(repast::AgentId id, Trade trade);
    virtual int getMarketData(MarketInfo mktdata);
    virtual Product* clone(string strProps) = 0;
};

class Stock : public Product
{
private:
    OrderBook ordBook;
    MarketInfo mktInfo;
    list<int> groups;

    Stock(MarketMaker *mkt, string stockPropsString);
    ~Stock();
    Product* clone(string productPropsString);

public:
    int matchOrder(Order order);
    int sendTrades(repast::AgentId id, Trade trade);
    MarketInfo *getMarketData();
    Product* clone(string productPropsString)
};

