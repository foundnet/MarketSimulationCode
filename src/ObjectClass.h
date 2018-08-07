#include <iostream>
#include <string>
#include "CommonTypes.h"

using namespace std;


class BaseObject
{

};

class Information : public BaseObject
{
    
};

class Product : public BaseObject
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