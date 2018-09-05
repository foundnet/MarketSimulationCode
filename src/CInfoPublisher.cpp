#include "CInfoPublisher.h"

InfoPublisher::InfoPublisher(repast::AgentId id, repast::Properties *agentProps) : BaseAgent(id)
{
    publisherName = agentProps->getProperty("publisher.name");
    string pubGroupsStr = agentProps->getProperty("publisher.groups");

    istringstream iss(pubGroupsStr);
    vector<string> params;
    do
    {
        string subs;
        iss >> subs;
        pubGroups.push_back(atoi(subs.c_str()));

    } while (iss);

    registerGroup();
}

BaseAgent* InfoPublisher::clone(repast::AgentId id, repast::Properties* agentProps)
{
    InfoPublisher *publisher = new InfoPublisher(id, agentProps);
    return publisher;
}

int InfoPublisher::messageProcessor(MessageInfo *info)
{
    
    return 1;
}

int InfoPublisher::handleStepWork()
{
    return 1;
}

//If group=-1 means pub to all, -2 means pub to all groups, other means send to a specified group
int InfoPublisher::PublishInfo(void *buff, int len, int sendflag)   
{
    if (sendflag == -1)
    {
        broadcastMessageInfo(buff, len, sendflag, MPI_COMM_WORLD);
    }
    else if (sendflag == -2)
    {
        for (int i = 0; i < pubGroups.size(); i++)
        {
            broadcastMessageInfo(buff, len, pubGroups[i], MPI_COMM_WORLD);
        }
    }
    else
    {
        int i;
        for (i = 0; i < pubGroups.size(); i++)
        {
            if (sendflag == pubGroups[i])
            {
                broadcastMessageInfo(buff, len, pubGroups[i], MPI_COMM_WORLD);
            }
        }

        if (i == pubGroups.size())     return 0;
    }

    return 1;
}

int InfoPublisher::registerGroup()
{
    for (int i=0; i<pubGroups.size() ;i++)
    {
        map<int, GroupInfo>::iterator iter;
        iter = model->groupMap.find(pubGroups[i]);
        if (iter != model->groupMap.end())
        {
            iter->second.isPub = 1;
            iter->second.members.push_back(id_);
        }
        else
        {
            GroupInfo grp;
            grp.members.push_back(id_);
            grp.isPub = true;
            model->groupMap[pubGroups[i]] = grp;
        }
    }
    return 1;
}
