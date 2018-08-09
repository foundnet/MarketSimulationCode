#include "Utilities.h"
class Agent {
public:
//Properties
    repast::AgentId   id_;  //Agent ID
    MsgRoundBuf msgQueue;
    boost::mpi::communicator comm;

//Actions
    Agent(repast::AgentId id);

    virtual repast::AgentId& getId(){                   return id_;    }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    virtual Agent* clone(repast::AgentId id) = 0;
    virtual int init();
    virtual int runStep() = 0;

    int sendPrivateInformation(repast::AgentId destAgentID, Information *info);
    int broadcastInformation(Information *info);
    int readInformationFromQueue(Information *info);
};