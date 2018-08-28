#include "CMarketMaker.h"


MarketMaker::MarketMaker(repast::AgentId id, repast::Properties *agentProps) : BaseAgent(id)
{
    marketName = agentProps->getProperty("market.name");
    productPropsFile = agentProps->getProperty("market.product.file");
}

MarketMaker::~MarketMaker()
{
    deleteProducts();
}

int MarketMaker::processOrder(Order *order)
{
    if (order != NULL)
    {
        Product *selProduct = NULL;
        selProduct = productMap[order->productID];
        if (selProduct != NULL)
        {
            selProduct->processOrder(*order);
            mktdataMap[order->productID] = *(selProduct->getMarketData());
            return 1;
        }
    }
    return 0;
}

BaseAgent* MarketMaker::clone(repast::AgentId id, repast::Properties *agentProps)
{
    MarketMaker *maker = new MarketMaker(id, agentProps);
    return maker;
}

int MarketMaker::handleInformation(Information *info)
{
    if (info->msgHead.msgType == 0)                 //If it's an order
    {
        processOrder((Order *)&info->body[0]);
        return 1;
    }
    return 0;
}

int MarketMaker::handleStepWork()
{
    return 1;
}

int MarketMaker::initProducts(Product *productPtr)
{
    ifstream file(productPropsFile);
    if (file.is_open())
    {
        std::string line;
        while (getline(file, line))
        {
            Product *product = productPtr->clone(this, line);
            productMap[product->productID] = product;
        }
        file.close();
        return 1;
    }
    return 0;
}

void MarketMaker::deleteProducts()
{
    unordered_map<int,Product*>::iterator iter;
    for (iter = productMap.begin(); iter != productMap.end(); iter++)
    {
        delete iter->second;
    }
    productMap.clear();
}

int MarketMaker::bcastMarketInfo(MarketInfo *mktInfo)
{
    return broadcastInformation((void *)mktInfo, sizeof(MarketInfo), 2, MPI_COMM_WORLD);
}
