#include "CAgent.h"

int Agent::init()  
{

    return 1;
}

Agent::Agent(repast::AgentId id): id_(id){ }

int Agent::readInformationFromQueue(Information *info)   {
    if (msgQueue.tail != msgQueue.head)
    {
        memcpy(info, &msgQueue.msg[msgQueue.tail], sizeof(ActorMessage));
        int point = (msgQueue.tail + 1) % ACTOR_MSGQUEUE_LEN;
        msgQueue.tail = point;
        return 1;
    }    
    return 0;
}

int Agent::broadcastInformation(Information *info)
{
    int dataLen = sizeof(Information) + info->bodyLength;
    if (dataLen > 64 || dataLen < 0)
        return 0;

    MPI_Request request;

    MPI_Ibcast(&info, 1, AF_ACTOR_TYPE, 0, comm , &request);
    MPI_Status status;
    MPI_Wait(&request, &status);

    return 1;
}

int Agent::sendPrivateInformation(repast::AgentId destAgentID, Information *info) {
    int dataLen = sizeof(Information) + info->bodyLength;
    if (dataLen >= 128 || dataLen < 0)
        return 0;
        
    MPI_Request request;
    MPI_Bsend(&info, 1, AF_ACTOR_TYPE, destAgentID->currentRank(), AF_ACTORMSG_TAG, comm);

    return 1;
}