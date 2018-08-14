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
    int productType;
    MarketMaker* market;
//Actions
    Product(MarketMaker* mkt, int pID, string pName,int pType):market(mkt),productID(pID),productName(pName),productType(pType){};
    virtual int matchOrder(Order order) = 0;
    virtual int sendTrades(repast::AgentId id, Trade trade);
    virtual int getMarketData(MarketInfo mktdata);
};

class Stock : public Product
{
private:
    OrderBook ordBook;
    MarketInfo mktInfo;
    list<int> groups;

    Stock(int stockID, string stockProps);
    ~Stock();

public:
    int matchOrder(Order order);
    int sendTrades(repast::AgentId id, Trade trade);
    MarketInfo *getMarketData();
};

