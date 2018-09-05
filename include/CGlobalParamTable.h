#include "CAgent.h"

class GlobalParamTable: public BaseAgent
{
public:
    GlobalParamTable();
    ~GlobalParamTable();

    map<string, map<double, string>> paramTable;
    map<int,MarketInfo> mktdataMap;

    


}