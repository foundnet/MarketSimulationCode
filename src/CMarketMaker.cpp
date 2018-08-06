#include <time.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>  
#include <list>

using namespace std;

typedef struct _Order {
    int orderNumber;
    int investorID;
    bool direction;             //Buy or Sell
    float price;
    int orderType;
    int count;
    int rank;
    timeval timeStamp;
} Order;

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

class StockExchange {
public:
    StockExchange(string cfgfile, int stockCount, int investorCount);
    ~StockExchange();
    int InputOrders(Order *pOrder,int cnt);
    int RegInvestors(InvestorInfo *pInfo, int count);
    int stockCnt;
    int investorCnt;
    int StartExchange();
    int CloseExchange();

    MarketInfo *market;
    OrderBook *orderbook;
    
    
private:
    int exchangeStatus=0;       //0-Before Start  1-Trading  2-Post Trade

};

StockExchange::StockExchange(string cfgfile, int stockCount, int investorCount) {
 
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

int StockExchange::StartExchange() {

}
StockExchange::~StockExchange() {
    delete market;
}

int StockExchange::InputOrders(Order *pOrder, int cnt) {
    if (exchangeStatus == 0) {
        exchangeStatus = 1;
        return 1;
    }
    else return 0;
}

int StockExchange::RegInvestors() {
    
}
