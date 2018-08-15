#ifndef COMMTYPE
#define COMMTYPE

#include <time.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>  


#include <list>
#include <map>
#include <vector>
#include <sys/time.h>
#include <unordered_map>
#include <bits/stl_pair.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>

#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/Point.h"

#include "repast_hpc/Schedule.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/AgentRequest.h"
#include "repast_hpc/TDataSource.h"
#include "repast_hpc/SVDataSet.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"



struct Informaion;

#define MAX_MSG_LEN 256

using namespace std;


struct _MessageHead
{
    repast::AgentId receiverID;
    repast::AgentId senderID;
    int msgType;
    int bodyLength;
} ;

typedef struct _Information
{
    _MessageHead msgHead;
    unsigned char body[MAX_MSG_LEN - sizeof(_MessageHead)];
} Information;


typedef struct _Order {
    int orderNumber;
    repast::AgentId agentID;    //Sender's ID
    bool direction;             //Buy or Sell
    int productID;              //ProductID of a financial product
    float price;
    int count;
    int orderType;         //0-limit order with storage 1-limit order with send back
    int rank;
    timeval timeStamp;
} Order;

typedef struct _OrderConfirm {
    int orderNumber;
    repast::AgentId agentID;
    bool direction;             //Buy or Sell
    int productID;              //ProductID of a financial product
    float price;
    int restCount;
    int operation;              //0-store in orderbook, 1-send back
    timeval timeStamp;
} OrderConfirm;

typedef struct _Trade {
    int orderNumber;
    //The ID of the counterParty
    repast::AgentId counterPartyID;
    int productID;
    bool direction;             //Buy or Sell
    float tradePrice;           
    int count;                  //The count of trade
    int rank;
    timeval timeStamp;
} Trade;


typedef struct _investorInfo {
    int investorID;
    int rank;       
    int stockID;
    int holding;
}InvestorInfo;

typedef struct _MarketInfo {
    int stockID;
    float lastPrice;
    float high;
    float low;
    timeval lastTradeTime;
    long volume;
    double turnover;
    int updateTimes = 0;
} MarketInfo;

typedef struct _OrderList {
    float price;
    list<Order> orders;
} OrderList;

typedef struct _OrderBook {
    int stockID;
    list<OrderList> BuyList;
    list<OrderList> SellList;
} OrderBook;

typedef struct _Holding {
    int productID;
    int count;
    int freezeCount;
    int holdingType;        //0-normal 
    timeval lastUpdateTime;
} Holding;


#define AGENT_MSGQUEUE_LEN 100
typedef struct _MsgRoundBuf
{
    int head;
    int tail;
    Information info[AGENT_MSGQUEUE_LEN];
    int pushInfo(Information *info) 
    {
        int point = (head + 1) % AGENT_MSGQUEUE_LEN;
        if (point != tail)
        {
            info[point] = *info;
            head = point;  
            return 1;
        }
        else return 0;
    };

} MsgRoundBuf ;

#endif