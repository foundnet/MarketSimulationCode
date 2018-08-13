#include "ObjectClass.h"

Stock::Stock(int stockID)
{
    ordBook.stockID = stockID;
    ordBook.BuyList.clear();
    ordBook.SellList.clear();
    memset(&mktInfo, 0, sizeof(MarketInfo))
}

int Stock::matchOrder(Order order)
{
    if (order.direction == true) //Buy
    {
        if (!ordBook.SellList.empty())
        {
            while (order.count > 0 && ordBook.SellList.front().price <= order.price)
            {
                Trade tradeActive, tradePassive;
                int reduceCount = order.count > ordBook.SellList.front().orders.front().count ? ordBook.SellList.front().orders.front().count : order.count;
                Trade tradeActive, tradePassive;
                tradeActive.count = reduceCount;
                tradeActive.direction = true;
                tradeActive.counterPartyID = ordBook.SellList.front().orders.front().agentID;
                tradeActive.rank = repast::RepastProcess::rank();
                tradeActive.timeStamp = gettimeofday(&tradeActive.timeStamp, NULL);
                tradeActive.orderNumber = order.orderNumber;
                tradeActive.tradePrice = ordBook.SellList.front().price;

                tradePassive.count = reduceCount;
                tradePassive.direction = false;
                tradePassive.counterPartyID = order.agentID;
                tradePassive.rank = repast::RepastProcess::rank();
                tradePassive.timeStamp = gettimeofday(&tradeActive.timeStamp, NULL);
                tradePassive.orderNumber = ordBook.SellList.front().orders.front().orderNumber;
                tradePassive.tradePrice = ordBook.SellList.front().price;

                sendTrades(order.agentID, tradeActive);
                sendTrades(ordBook.SellList.front().orders.front().agentID, tradePassive);
 
                mktInfo.stockID = ordBook.stockID;
                mktInfo.lastPrice = ordBook.SellList.front().price;
                mktInfo.high = mktInfo.lastPrice > mktInfo.high?mktInfo.lastPrice:mktInfo.high;
                if (mktInfo.low == 0)   mktInfo.low = mktInfo.lastPrice;
                else  mktInfo.low = mktInfo.lastPrice < mktInfo.low?mktInfo.lastPrice:mktInfo.low;
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
        }
    }
    else //Sell
    {
        if (!ordBook.BuyList.empty())
        {
            while (order.count > 0 && ordBook.BuyList.front().price >= order.price)
            {
                Trade tradeActive, tradePassive;
                int reduceCount = order.count > ordBook.BuyList.front().orders.front().count ? ordBook.BuyList.front().orders.front().count : order.count;
                Trade tradeActive, tradePassive;
                tradeActive.count = reduceCount;
                tradeActive.direction = false;
                tradeActive.counterPartyID = ordBook.BuyList.front().orders.front().agentID;
                tradeActive.rank = repast::RepastProcess::rank();
                tradeActive.timeStamp = gettimeofday(&tradeActive.timeStamp, NULL);
                tradeActive.orderNumber = order.orderNumber;                tradeActive.rank = repast::RepastProcess::rank();

                tradeActive.tradePrice = ordBook.BuyList.front().price;

                tradePassive.count = reduceCount;
                tradePassive.direction = false;
                tradePassive.counterPartyID = order.agentID;
                tradePassive.rank = repast::RepastProcess::rank();
                tradePassive.timeStamp = tradeActive.timeStamp;
                tradePassive.orderNumber = ordBook.BuyList.front().orders.front().orderNumber;
                tradePassive.tradePrice = ordBook.BuyList.front().price;

                sendTrades(order.agentID, tradeActive);
                sendTrades(ordBook.BuyList.front().orders.front().agentID, tradePassive);

                mktInfo.stockID = ordBook.stockID;
                mktInfo.lastPrice = ordBook.BuyList.front().price;
                mktInfo.high = mktInfo.lastPrice > mktInfo.high?mktInfo.lastPrice:mktInfo.high;
                if (mktInfo.low == 0)   mktInfo.low = mktInfo.lastPrice;
                else  mktInfo.low = mktInfo.lastPrice < mktInfo.low?mktInfo.lastPrice:mktInfo.low;
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
        }
    }
}

int Stock::sendTrades(repast::AgentId id, Trade trade)
{
    market->sendPrivateInformation(id, (unsigned char*)trade, sizeof(Trade), 0);
    //Todo: we can add the subscriber list, because someone would be interested in other's trades
}

MarketInfo *Stock::getMarketData()
{
    return &mktInfo;
}