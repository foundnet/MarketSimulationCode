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

   	std::cout << id.id() << "    " << currency << "   "<< repast::RepastProcess::instance()->rank() << std::endl;

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
                    int min = atoi(params[1].c_str());
                    int max = atoi(params[2].c_str());
                    if (min == max) 
                        hold.count = min;
                    else
                        hold.count = repast::Random::instance()->nextDouble() * (max-min) + min;
                    hold.freezeCount = 0;
                    hold.holdingType = 0;
                    holdingMap[hold.productID] = hold;
                   	std::cout << hold.productID << "/" << hold.count << "/"<< repast::RepastProcess::instance()->rank() << std::endl;
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

int MktParticipant::MessageProcessor(MessageInfo *info)
{
    if (info->msgHead.msgType == 1)                 //If it's a trade
    {
        TradeResult *trade = (TradeResult*)info->body;
        if (trade->direction)
            holdingMap[trade->productID].count = holdingMap[trade->productID].count + trade->count;
        else  
            holdingMap[trade->productID].count = holdingMap[trade->productID].count - trade->count;
        
        pastTrades.push_back(*trade);
    }
    else if (info->msgHead.msgType == 2)             //If it's a orderConfirm
    {
        TradeResult * ordCnfm = (TradeResult*)info->body;
        if (ordCnfm->operation == 1)
        {
            if (ordCnfm->direction)
            {
                currency += ordCnfm->price * ordCnfm->count;
            }
            else
            {
                holdingMap[ordCnfm->productID].count =holdingMap[ordCnfm->productID].count + ordCnfm->count;
            }
        } 
    }
    else if (info->msgHead.msgType == 3)             //If it's a market info
    {
        MarketInfo *mktInfo = (MarketInfo *)info->body;
        mktdataMap[mktInfo->stockID] = *(mktInfo);
    }
    else return BaseAgent::MessageProcessor(info);

    return 1;
}


int MktParticipant::handleStepWork()
{
    conditionCheck();
    actionExecuter();
    expectationGenerator();
    
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
            sendPrivateMessageInfo(markets[j]->getId(), (unsigned char *)&order, sizeof(Order), 0);
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

int MktParticipant::conditionCheck()
{
    map<string, Condition>::iterator iter = conditionMap.begin();
    while (iter != conditionMap.end())
    {
        iter->second.curStatus = iter->second.conFunctionPtr(&paramTable, &paramTable);
    }
    return 1;
}

int MktParticipant::actionExecuter()
{
    list<Rule>::iterator iter = ruleTable.begin();
    while (iter != ruleTable.end())
    {
        list<ConStatus>::iterator iterCon = iter->conditionSet.begin();
        while (iterCon != iter->conditionSet.end())
        {
            if ( !conditionMap[iterCon->conName].curStatus)
                break;
        }
        if (iterCon == iter->conditionSet.end()) iter->action(&paramTable, &paramTable);
    }
    return 1;
}

int MktParticipant::expectationGenerator()
{
    return 1;
}