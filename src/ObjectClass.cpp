#include "ObjectClass.h"
#include "CMarketMaker.h"

Stock::Stock(MarketMaker *mkt, string stockPropsString):Product(mkt)
{
    istringstream iss(stockPropsString);
    vector<string> params;
    do
    {
        string subs;           
        iss >> subs;
        params.push_back(subs);
    } while (iss);
    if (params.size() > 2)
    {
        productID = atoi(params[0].c_str());
        productName = params[1];
        productType = params[2];

        ordBook.stockID = productID;
        ordBook.BuyList.clear();
        ordBook.SellList.clear();
        memset(&mktInfo, 0, sizeof(MarketInfo));
    }
}

int Stock::processOrder(Order order)
{
    if (order.direction    == true) //Buy order
    {
        if (!ordBook.SellList.empty())
        {
            while (order.count > 0 && ordBook.SellList.front().price <= order.price)
            {
                TradeResult tradeActive, tradePassive;
                int reduceCount = order.count > ordBook.SellList.front().orders.front().count ? ordBook.SellList.front().orders.front().count : order.count;
                tradeActive.count = reduceCount;
                tradeActive.resultType = 1;
                tradeActive.direction = true;
                tradeActive.counterPartyID = ordBook.SellList.front().orders.front().agentID;
                tradeActive.agentID = order.agentID;
                tradeActive.marketRank = repast::RepastProcess::instance()->rank();
                gettimeofday(&tradeActive.timeStamp, NULL);
                tradeActive.orderNumber = order.orderNumber;
                tradeActive.price = ordBook.SellList.front().price;

                tradePassive.count = reduceCount;
                tradePassive.direction = false;
                tradePassive.counterPartyID = order.agentID;
                tradePassive.agentID = ordBook.SellList.front().orders.front().agentID;
                tradePassive.resultType = 1;
                tradePassive.marketRank = repast::RepastProcess::instance()->rank();
                tradePassive.timeStamp = tradeActive.timeStamp;
                tradePassive.orderNumber = ordBook.SellList.front().orders.front().orderNumber;
                tradePassive.price = ordBook.SellList.front().price;

                putTradeResult(order.agentID, tradeActive);
                putTradeResult(ordBook.SellList.front().orders.front().agentID, tradePassive);

                mktInfo.stockID = ordBook.stockID;
                mktInfo.lastPrice = ordBook.SellList.front().price;
                mktInfo.high = mktInfo.lastPrice > mktInfo.high ? mktInfo.lastPrice : mktInfo.high;
                if (mktInfo.low == 0)
                    mktInfo.low = mktInfo.lastPrice;
                else
                    mktInfo.low = mktInfo.lastPrice < mktInfo.low ? mktInfo.lastPrice : mktInfo.low;
                mktInfo.lastTradeTime = tradePassive.timeStamp;
                mktInfo.volume += reduceCount;
                mktInfo.turnover += reduceCount * mktInfo.lastPrice;
                mktInfo.updateTimes++;

                order.count -= reduceCount;
                ordBook.SellList.front().orders.front().count -= reduceCount;
                if (ordBook.SellList.front().orders.front().count == 0)
                {
                    ordBook.SellList.front().orders.pop_front();
                    if (ordBook.SellList.front().orders.size() == 0)
                        ordBook.SellList.pop_front();
                }
            }
        }
        if (order.count > 0)
        {
            TradeResult trdResult;
            if (order.orderType == 0)
            {
                bool isProcessed = false;
                if (!ordBook.BuyList.empty())
                {
                    for (list<OrderList>::iterator iter = ordBook.BuyList.begin(); iter != ordBook.BuyList.end(); iter++)
                    {
                        if (iter->price > order.price)
                            continue;

                        if (iter->price == order.price)
                            iter->orders.push_back(order);
                        else if (iter->price < order.price)
                        {
                            OrderList newList;
                            newList.price = order.price;
                            newList.orders.push_back(order);
                            ordBook.BuyList.insert(iter, newList);
                        }
                        isProcessed = true;
                        break;
                    }
                }
                if (!isProcessed)
                {
                    OrderList newList;
                    newList.price = order.price;
                    newList.orders.push_back(order);
                    ordBook.BuyList.push_back(newList);
                }
                trdResult.operation = 0;              //0-store in orderbook, 1-send back
            }
            else trdResult.operation = 1; 

            trdResult.orderNumber = order.orderNumber;
            trdResult.counterPartyID = order.agentID;
            trdResult.direction = order.direction;             //Buy or Sell
            trdResult.productID = order.productID;              //ProductID of a financial product
            trdResult.price = order.price;
            trdResult.count = order.count;
            gettimeofday(&trdResult.timeStamp, NULL);
            trdResult.resultType = 2;
            putTradeResult(trdResult.counterPartyID, trdResult);
        }
    }
    else //Sell order
    {
        if (!ordBook.BuyList.empty())
        {
            while (order.count > 0 && ordBook.BuyList.front().price >= order.price)
            {
                TradeResult tradeActive, tradePassive;
                int reduceCount = order.count > ordBook.BuyList.front().orders.front().count ? ordBook.BuyList.front().orders.front().count : order.count;
                tradeActive.count = reduceCount;
                tradeActive.direction = false;
                tradeActive.resultType = 1;
                tradeActive.counterPartyID = ordBook.BuyList.front().orders.front().agentID;
                tradeActive.marketRank = repast::RepastProcess::instance()->rank();
                gettimeofday(&tradeActive.timeStamp, NULL);
                tradeActive.orderNumber = order.orderNumber;

                tradeActive.price = ordBook.BuyList.front().price;

                tradePassive.count = reduceCount;
                tradePassive.direction = false;
                tradePassive.counterPartyID = order.agentID;
                tradePassive.resultType = 1;
                tradePassive.marketRank = repast::RepastProcess::instance()->rank();
                tradePassive.timeStamp = tradeActive.timeStamp;
                tradePassive.orderNumber = ordBook.BuyList.front().orders.front().orderNumber;
                tradePassive.price = ordBook.BuyList.front().price;

                putTradeResult(order.agentID, tradeActive);
                putTradeResult(ordBook.BuyList.front().orders.front().agentID, tradePassive);

                mktInfo.stockID = ordBook.stockID;
                mktInfo.lastPrice = ordBook.BuyList.front().price;
                mktInfo.high = mktInfo.lastPrice > mktInfo.high ? mktInfo.lastPrice : mktInfo.high;
                if (mktInfo.low == 0)
                    mktInfo.low = mktInfo.lastPrice;
                else
                    mktInfo.low = mktInfo.lastPrice < mktInfo.low ? mktInfo.lastPrice : mktInfo.low;
                mktInfo.lastTradeTime = tradePassive.timeStamp;
                mktInfo.volume += reduceCount;
                mktInfo.turnover += reduceCount * mktInfo.lastPrice;
                mktInfo.updateTimes++;

                order.count -= reduceCount;
                ordBook.BuyList.front().orders.front().count -= reduceCount;
                if (ordBook.BuyList.front().orders.front().count == 0)
                {
                    ordBook.BuyList.front().orders.pop_front();
                    if (ordBook.BuyList.front().orders.size() == 0)
                        ordBook.BuyList.pop_front();
                }
            }
        }
        if (order.count > 0)
        {
            TradeResult trdResult;
            if (order.orderType == 0)
            {
                bool isProcessed = false;
                if (!ordBook.SellList.empty())
                {
                    for (list<OrderList>::iterator iter = ordBook.SellList.begin(); iter != ordBook.SellList.end(); iter++)
                    {
                        if (iter->price < order.price)
                            continue;

                        if (iter->price == order.price)
                            iter->orders.push_back(order);
                        else if (iter->price > order.price)
                        {
                            OrderList newList;
                            newList.price = order.price;
                            newList.orders.push_back(order);
                            ordBook.SellList.insert(iter, newList);
                        }
                        isProcessed = true;
                        break;
                    }
                }
                if (!isProcessed)
                {
                    OrderList newList;
                    newList.price = order.price;
                    newList.orders.push_back(order);
                    ordBook.SellList.push_back(newList);
                }

                trdResult.operation = 0;              //0-store in orderbook, 1-send back
            }
            else trdResult.operation = 1; 

            trdResult.orderNumber = order.orderNumber;
            trdResult.counterPartyID = order.agentID;
            trdResult.direction = order.direction;             //Buy or Sell
            trdResult.productID = order.productID;              //ProductID of a financial product
            trdResult.price = order.price;
            trdResult.count = order.count;
            trdResult.resultType = 2;
            gettimeofday(&trdResult.timeStamp, NULL);

            putTradeResult(order.agentID, trdResult);
        }
    }
    return 1;
}

int Stock::putTradeResult(repast::AgentId id, TradeResult trade)
{
    trdResultList.push_back(trade);
    //Todo: we can add the subscriber list, because someone would be interested in other's orderconfirm
}

TradeResult *Stock::getTradeResult()
{
    if (!trdResultList.empty())
    {
        TradeResult *trade = new TradeResult();
        *trade = trdResultList.front();
        trdResultList.pop_front();
        return trade;
    }

    return NULL;

    //Todo: we can add the subscriber list, because someone would be interested in other's orderconfirm
}


MarketInfo *Stock::getMarketData()
{
    return &mktInfo;
}

Product *Stock::clone(MarketMaker *mkt, string productPropsString)
{
    Stock *stock = new Stock(mkt, productPropsString);
    return (Product *)stock;
}
