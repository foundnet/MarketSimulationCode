
#ifndef FRAMEMODEL
#define FRAMEMODEL


#include "ObjectClass.h"
#include "CAgent.h"

class FrameworkModel{
public:	
	FrameworkModel(string propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	~FrameworkModel();

	int stopAt;
	int startX, startY, lengthX, lengthY;
	MPI_Request privateRequest = NULL;
	MPI_Request bcastRequest = NULL;
	boost::mpi::communicator *comm;

	Information privateInfo;
	Information bcastInfo;

	vector<int> agentTypes;
	repast::Properties* props;
	repast::SharedContext<BaseAgent> context;
	
    repast::SharedDiscreteSpace<BaseAgent, repast::WrapAroundBorders, repast::SimpleAdder<BaseAgent>>* discreteSpace;
	
	int DispatchInformation(Information *info);
	void MessagePoll();
	int initAgents(BaseAgent *agentPtr, string agentPropsFile);
	void runStep();
	void initSchedule(repast::ScheduleRunner& runner);
};

#endif
