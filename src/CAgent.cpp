#include "CAgent.h"

int BaseAgent::readInformationFromQueue(Information *info)   {
    if (msgQueue.tail != msgQueue.head)
    {
        memcpy(info, &msgQueue.info[msgQueue.tail], sizeof(Information));
        int point = (msgQueue.tail + 1) % AGENT_MSGQUEUE_LEN;
        msgQueue.tail = point;
        return 1;
    }    
    return 0;
}

int BaseAgent::broadcastInformation(void *buff, int len, int msgType, MPI_Comm groupComm)
{
    if (len > (MAX_MSG_LEN-sizeof(_MessageHead)) || len < 1)  return 0;    

    Information info;
    info.msgHead.senderID = id_;
    info.msgHead.receiverID = id_;
    info.msgHead.msgType = msgType;
    info.msgHead.bodyLength = len;
    memcpy(&info.body, buff, len);

    MPI_Request request;

    MPI_Ibcast(&info, len+sizeof(_MessageHead), MPI_UNSIGNED_CHAR, id_.currentRank(),groupComm , &request);

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
    MPI_Bsend(&info, len + sizeof(_MessageHead), MPI_UNSIGNED_CHAR, destAgentID.currentRank(), 0, MPI_COMM_WORLD);

    return 1;
}

int BaseAgent::runStep()
{
    Information info;
    
    while (readInformationFromQueue(&info))
        handleInformation(&info);
    
    handleStepWork();
    return 1;
}


int BaseAgent::init(repast::SharedContext<BaseAgent> *ctx)
{
    context = ctx;
}
