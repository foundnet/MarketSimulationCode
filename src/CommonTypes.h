#include <time.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>  
#include <list>
#include <map>
#include<vector>



using namespace std;

typedef struct _Order {
    int orderNumber;
    int ParticipantID;
    bool direction;             //Buy or Sell
    float price;
    int orderType;
    int count;
    int rank;
    timeval timeStamp;
} Order;

typedef struct _Trade {
    int orderNumber;
    int counterPartyID;         //The ID of the counterParty
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
    int count;
    int holdingType;
    int holdingDate;
} Holding;
