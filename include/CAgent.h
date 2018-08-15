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
    repast::SharedContext<BaseAgent> *context;
    
//Actions   
    BaseAgent(repast::AgentId id):id_(id){};
    virtual ~BaseAgent() {};

    repast::AgentId & getId(){   return id_;  }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    virtual BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps) = 0;
    virtual int init(repast::SharedContext<BaseAgent> *ctx);
    virtual int runStep();
    virtual int handleInformation(Information *info) = 0;
    virtual int handleStepWork() = 0;

    int broadcastInformation(void *buff, int len, int msgType, MPI_Comm groupComm);
    int sendPrivateInformation(repast::AgentId destAgentID, unsigned char *buff, int len, int msgType);
    int readInformationFromQueue(Information *info);
    
};


#endif