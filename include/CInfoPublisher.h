#ifndef INFOPUBLISHER
#define INFOPUBLISHER

#include "ObjectClass.h"
#include "CAgent.h"

using namespace std;
class InfoPublisher : public BaseAgent
{
public:
//Properties
    vector<int> pubGroups;             //Publisher's Group
    string publisherName;
    int pubMethod;          //0-Broadcast 1-Inside Category 2-Inside the same agent type
//Actions
    InfoPublisher(repast::AgentId id, repast::Properties *agentProps);
    ~InfoPublisher(){};

    BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps);
    int MessageProcessor(MessageInfo *info);
    int handleStepWork();

    int PublishInfo(void *buff, int len, int sendflag);

};



#endif











































































































