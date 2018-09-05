#ifndef INFORECEIVER
#define INFORECEIVER

#include "ObjectClass.h"
#include "CAgent.h"

using namespace std;
class InfoReceiver : public BaseAgent
{
public:
//Properties
    vector<int> rcvGroups;             //Receiver's Group
    string receiverName;

//Actions
    InfoReceiver(repast::AgentId id, repast::Properties *agentProps);
    ~InfoReceiver(){};

    BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps);
    int messageProcessor(MessageInfo *info);
    int registerGroup();
    int handleStepWork();

};



#endif





