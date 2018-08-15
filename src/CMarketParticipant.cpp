#include "CMarketParticipant.h"
#include "CMarketMaker.h"

MktParticipant::MktParticipant(repast::AgentId id, repast::Properties *agentProps) : BaseAgent(id)
{
    category = repast::strToInt(agentProps->getProperty("participant.category"));

    int maxCurrency = repast::strToInt(agentProps->getProperty("participant.max_currency"));
    int minCurrency = repast::strToInt(agentProps->getProperty("participant.min_currency"));

    if (maxCurrency == minCurrency && maxCurrency > 0)
        currency = maxCurrency;
    else 
    {
        currency = repast::Random::instance()->nextDouble() * (maxCurrency-minCurrency) + minCurrency;
    }

    string holdingFile = agentProps->getProperty("participant.holding.file");
    
    if (!holdingFile.empty())
    {
        ifstream file(holdingFile);
        if (file.is_open())
        {
            std::string line;
            while (getline(file, line))
            {
                istringstream iss(line);
                vector<string> params;
                do
                {
                    string subs;
                    iss >> subs;
                    params.push_back(subs);
                } while (iss);
                if (params.size() >1)
                {
                    Holding hold;
                    hold.productID = atoi(params[0].c_str());
                    hold.count = atoi(params[1].c_str());
                    hold.freezeCount = 0;
                    hold.holdingType = 0;
                    holdingMap[hold.productID] = hold;
                }
            }
            file.close();
        }
    }
}

MktParticipant::~MktParticipant()
{
    ;
}

BaseAgent* MktParticipant::clone(repast::AgentId id, repast::Properties* agentProps)
{
    MktParticipant *participant = new MktParticipant(id, agentProps);
    return participant;
}

int MktParticipant::handleInformation(Information *info)
{
    if (info->msgHead.msgType == 1)                 //If it's a trade
    {
        Trade *trade = (Trade*)info->body;
        if (trade->direction)
            holdingMap[trade->productID].count = holdingMap[trade->productID].count + trade->count;
        else  
            holdingMap[trade->productID].count = holdingMap[trade->productID].count - trade->count;
        
        pastTrades.push_back(*trade);
    }
    else if (info->msgHead.msgType == 2)             //If it's a orderConfirm
    {
        OrderConfirm * ordCnfm = (OrderConfirm*)info->body;
        if (ordCnfm->operation == 1)
        {
            if (ordCnfm->direction)
            {
                currency += ordCnfm->price * ordCnfm->restCount;
            }
            else
            {
                holdingMap[ordCnfm->productID].count ==holdingMap[ordCnfm->productID].count + ordCnfm->restCount;
            }
        } 
    }
    else if (info->msgHead.msgType == 3)             //If it's a market info
    {
        MarketInfo *mktInfo = (MarketInfo *)info->body;
        mktdataMap[mktInfo->stockID] = *(mktInfo);
    }
    else return 0;

    return 1;
}


int MktParticipant::handleStepWork()
{
    
    return 1;
}

void MktParticipant::registerMarkets(vector<MarketMaker*> mkt)
{
    markets = mkt;
}

int MktParticipant::sendOrders(Order *order)
{
	for(size_t j = 0; j < markets.size(); j++)
    {
	    if (markets[j]->productMap.count(order->productID) > 0)
        {
            sendPrivateInformation(markets[j]->getId(), (unsigned char *)&order, sizeof(Order), 0);
            if (order->direction)
                currency -= order->count * order->price;
            else 
                holdingMap[order->productID].count = holdingMap[order->productID].count - order->count;
        }

        return 1;
    }
    return 0;
    //Todo: we can add the subscriber list, because someone would be interested in other's trades
}

int MktParticipant::calculateExpectation(int productID)      
{
    //To do: override it to customize the expectation calculation 
    int expectedPrice = repast::Random::instance()->nextDouble()*100;

    return expectedPrice;
}


