#include <iostream>
#include <string>

#include "CAgent.h"
#include "ObjectClass.h"

using namespace std;
class InfoPublisher : public BaseMktAgent
{
public:
//Properties
    string name;
    int categoryID;             //Publisher's Category
    int defaultPubMethod;          //0-Broadcast 1-Inside Category 2-Inside the same agent type
//Actions
    //to do
    int PublishInfo(Information info, int publishMethod);

};

















































































































