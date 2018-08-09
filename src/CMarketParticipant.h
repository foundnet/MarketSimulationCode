
#include <string>

#include "CAgent.h"
#include "CommonTypes.h"

using namespace std;

class MktParticipant : public Agent {
public:
//Properties
    string name;
    int categoryID;
    map<string, Holding> holdingMap;            //The holding info
    vector<Trade>  pastTrades;
    int currencyAmount;
    int currencyType;
//Actions
    int SendOrders(int mktMakerID, Order order);
    int ReceivePrivateInfo();
};