#include "CAgent.h"


int BaseAgent::readMessageInfoFromQueue(MessageInfo *info)   {
    if (msgQueue.tail != msgQueue.head)
    {
        memcpy(info, &msgQueue.info[msgQueue.tail], sizeof(MessageInfo));
        int point = (msgQueue.tail + 1) % AGENT_MSGQUEUE_LEN;
        msgQueue.tail = point;
        return 1;
    }    
    return 0;
}

int BaseAgent::broadcastMessageInfo(void *buff, int len, int msgType, MPI_Comm groupComm)
{
    if (len > (MAX_MSG_LEN-sizeof(_MessageHead)) || len < 1)  return 0;    

    MessageInfo info;
    info.msgHead.senderID = id_;
    info.msgHead.receiverID = id_;
    info.msgHead.msgType = msgType;
    info.msgHead.bodyLength = len;
    memcpy(&info.information, buff, len);

    MPI_Request request;

    MPI_Ibcast(&info, len+sizeof(_MessageHead), MPI_UNSIGNED_CHAR, id_.currentRank(),groupComm , &request);

    MPI_Status status;
    MPI_Wait(&request, &status);

    return 1;
}

int BaseAgent::sendPrivateMessageInfo(repast::AgentId destAgentID, unsigned char *buff, int len, int msgType) {
    if (len > (MAX_MSG_LEN-sizeof(_MessageHead)) || len < 1)  return 0;    
 
    MessageInfo info;
    info.msgHead.senderID = id_;
    info.msgHead.receiverID = destAgentID;
    info.msgHead.msgType = msgType;
    info.msgHead.bodyLength = len;
    memcpy(&info.information, buff, len);

    MPI_Request request;
    MPI_Bsend(&info, len + sizeof(_MessageHead), MPI_UNSIGNED_CHAR, destAgentID.currentRank(), 0, MPI_COMM_WORLD);

    return 1;
}

int BaseAgent::runStep()
{
    MessageInfo info;
    
    while (readMessageInfoFromQueue(&info))
        messageProcessor(&info);
    
    handleStepWork();
    return 1;
}


int BaseAgent::init(FrameworkModel *m)
{
    context = &m->context;
    model = m;
  	std::cout << "BaseAgent:init id:" << id_.id() << " rank:" << repast::RepastProcess::instance()->rank() << std::endl;
}

int BaseAgent::message2Param(MessageInfo *info)
{
    Json::Value root;
    Json::Reader reader;

    string strJson = (const char*)info->information;

    if (reader.parse(strJson, root))
    {
        double curTick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
        for (int i=0; i<root.size(); i++)
        {
            vector<string> vNames = root.getMemberNames();
            for (int i = 0; i<vNames.size(); i++)
            {
                paramTable[vNames[i]][curTick] = root[vNames[i]].asString();
            }  
        }
    }

}
