#include "CAgent.h"

int BaseAgent::init()  
{

    return 1;
}

BaseAgent::BaseAgent(repast::AgentId id): id_(id){ }

int BaseAgent::readInformationFromQueue(Information *info)   {
    if (msgQueue.tail != msgQueue.head)
    {
        memcpy(info, &msgQueue.msg[msgQueue.tail], sizeof(ActorMessage));
        int point = (msgQueue.tail + 1) % ACTOR_MSGQUEUE_LEN;
        msgQueue.tail = point;
        return 1;
    }    
    return 0;
}

int BaseAgent::broadcastInformation(void *buff, int len, int msgType)
{
    if (len > (MAX_MSG_LEN-sizeof(_MessageHead)) || len < 1)  return 0;    

    Information info;
    info.msgHead.senderID = id_;
    info.msgHead.receiverID = id_;
    info.msgHead.msgType = msgType;
    info.msgHead.bodyLength = len;
    memcpy(&info.body, buff, len);

    MPI_Request request;

    MPI_Ibcast(&info, len+sizeof(_MessageHead), MPI_UNSIGNED_CHAR, id_.currentRank(), comm , &request);

    MPI_Status status;
    MPI_Wait(&request, &status);

    return 1;
}

int BaseAgent::sendPrivateInformation(repast::AgentId destAgentID, unsigned char *buff, int len, int msgType) {
    if (len > (MAX_MSG_LEN-sizeof(_MessageHead)) || len < 1)  return 0;    
 
    Information info;
    info.msgHead.senderID = id_;
    info.msgHead.receiverID = destAgentID;
    info.msgHead.msgType = msgType;
    info.msgHead.bodyLength = len;
    memcpy(&info.body, buff, len);

    MPI_Request request;
    MPI_Bsend(&info, len + sizeof(_MessageHead), MPI_UNSIGNED_CHAR, destAgentID.currentRank(), 0, comm);

    return 1;
}