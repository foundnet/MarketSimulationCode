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

    registerGroup();

}

BaseAgent* InfoReceiver::clone(repast::AgentId id, repast::Properties* agentProps)
{
    InfoReceiver *receiver = new InfoReceiver(id, agentProps);
    return receiver;
}

int InfoReceiver::messageProcessor(MessageInfo *info)
{
    return 1;
}

int InfoReceiver::handleStepWork()
{
    return 1;
}

int InfoReceiver::registerGroup()
{
    for (int i=0; i<rcvGroups.size() ;i++)
    {
        map<int, GroupInfo>::iterator iter;
        iter = model->groupMap.find(rcvGroups[i]);
        if (iter != model->groupMap.end())
        {
            iter->second.members.push_back(id_);
        }
        else
        {
            GroupInfo grp;
            grp.members.push_back(id_);
            grp.isPub = false;
            model->groupMap[rcvGroups[i]] = grp;
        }
    }
    return 1;
}