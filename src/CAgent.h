#include "CommonTypes.h"
#include "ObjectClass.h"
#include "Utilities.h"


class BaseAgent {
private:
//Properties
    repast::AgentId id_;  //Agent ID
    boost::mpi::communicator comm_;
public:
    int group;
    MsgRoundBuf msgQueue;
    
//Actions   
    BaseAgent(repast::AgentId id);
    virtual ~BaseAgent();

    repast::AgentId & getId(){                   return id_;}
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    virtual BaseAgent* clone(repast::AgentId id, repast::Properties* agentProps) = 0;
    virtual int init();
    virtual int runStep() = 0;

    int broadcastInformation(void *buff, int len, int msgType, MPI_Comm groupComm);
    int sendPrivateInformation(repast::AgentId destAgentID, unsigned char *buff, int len, int msgType);
    int readInformationFromQueue(Information *info);

};