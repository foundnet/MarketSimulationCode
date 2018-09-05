#ifndef BASEAGENT
#define BASEAGENT

#include "ObjectClass.h"
#include "FrameworkModel.h"


class BaseAgent {
public:
//Properties
    repast::AgentId id_;  //Agent ID
    boost::mpi::communicator comm_;
    MsgRoundBuf msgQueue;

    int group[MAX_GROUP_CNT] = {0};
    map<string, map<double, string>> paramTable;
 
    FrameworkModel *model;
    repast::SharedContext<BaseAgent> *context; 
    
//Actions   
    BaseAgent(){};
    BaseAgent(repast::AgentId id):id_(id){};

    virtual ~BaseAgent() {};

    repast::AgentId & getId(){   return id_;  }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    virtual BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps) = 0;
    virtual int init(FrameworkModel *m);
    virtual int runStep();
    virtual int messageProcessor(MessageInfo *info) = 0;
    virtual int handleStepWork() = 0;
    int message2Param(MessageInfo *info);


    int broadcastMessageInfo(void *buff, int len, int msgType, MPI_Comm groupComm);
    int sendPrivateMessageInfo(repast::AgentId destAgentID, unsigned char *buff, int len, int msgType);
    int readMessageInfoFromQueue(MessageInfo *info);
    
};


#endif