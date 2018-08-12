#include <iostream>
#include <string>
#include "CommonTypes.h"

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

struct Product : BaseObject
{
public:
//Properties
    int productID;
    string productName;
    int productType;
//Actions
    int MatchOrder(Order order);
    int SendPrivateInfo(Information info);
    int GetMktdata(MarketInfo mktdata);
};

class Stock : public Product
{
public:

};

