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

int processOrder(Order *order);
{

}

BaseAgent *MarketMaker::clone(repast::AgentId id, repast::Properties *agentProps)
{
    MarketMaker *maker = new MarketMaker(id, agentProps);
    return maker;
}

int MarketMaker::runStep()
{
    return 0;
}

int MarketMaker::initProducts(Product *productPtr)
{
    ifstream file(productPropsFile);
    if (file.is_open())
    {
        std::string line;
        while (getline(file, line))
        {
            Product *product = productPtr->clone(line);
            products.insert(std::make_pair<int,Product*>(product->productID,product));
        }
        file.close();
    }
}

int MarketMaker::deleteProducts()
{
    unordered_map<int,Product*>::iterator iter;
    for (iter = productMap.begin(); iter != productMap.end(); iter++)
    {
        delete iter->second;
    }
    productMap.clear();
}
