#include "CInfoReceiver.h"

InfoReceiver::InfoReceiver(repast::AgentId id, repast::Properties *agentProps) : BaseAgent(id)
{
    receiverName = agentProps->getProperty("receiver.name");
    string rcvGroupStr = agentProps->getProperty("receiver.groups");

    istringstream iss(rcvGroupStr);
    vector<string> params;
    do
    {
        string subs;
        iss >> subs;
        rcvGroups.push_back(atoi(subs.c_str()));

    } while (iss);
}

BaseAgent* InfoReceiver::clone(repast::AgentId id, repast::Properties* agentProps)
{
    InfoReceiver *receiver = new InfoReceiver(id, agentProps);
    return receiver;
}

int InfoReceiver::handleInformation(Information *info)
{
    return 1;
}

int InfoReceiver::handleStepWork()
{
    return 1;
}
