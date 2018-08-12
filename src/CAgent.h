#include "CommonTypes.h"
#include "ObjectClass.h"
#include "Utilities.h"


class BaseAgent {
private:
//Properties
    repast::AgentId   id_;  //Agent ID
    MsgRoundBuf msgQueue;
    boost::mpi::communicator comm;
public:
//Actions   
    BaseAgent::BaseAgent(repast::AgentId id);

    repast::AgentId & getId(){                   return id_;}
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    virtual BaseAgent* clone(repast::AgentId id) = 0;
    virtual int init();
    virtual int runStep() = 0;

    int sendPrivateInformation(repast::AgentId destAgentID, Information *info);
    int broadcastInformation(Information *info);
    int readInformationFromQueue(Information *info);
};