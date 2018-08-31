#ifndef BASEAGENT
#define BASEAGENT

#include "ObjectClass.h"


class BaseAgent {
public:
//Properties
    repast::AgentId id_;  //Agent ID
    boost::mpi::communicator comm_;

    int group;
    MsgRoundBuf msgQueue;
    map<string, map<double, string>> paramTable;

    repast::SharedContext<BaseAgent> *context;
    
    
//Actions   
    BaseAgent(){};
    BaseAgent(repast::AgentId id):id_(id){};

    virtual ~BaseAgent() {};

    repast::AgentId & getId(){   return id_;  }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    virtual BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps) = 0;
    virtual int init(repast::SharedContext<BaseAgent> *ctx);
    virtual int runStep();
    virtual int MessageProcessor(MessageInfo *info);
    virtual int handleStepWork() = 0;

    int broadcastMessageInfo(void *buff, int len, int msgType, MPI_Comm groupComm);
    int sendPrivateMessageInfo(repast::AgentId destAgentID, unsigned char *buff, int len, int msgType);
    int readMessageInfoFromQueue(MessageInfo *info);
    
};


#endif