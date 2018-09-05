
#ifndef FRAMEMODEL
#define FRAMEMODEL


#include "ObjectClass.h"
#include "CAgent.h"

typedef struct _GroupInfo
{
	bool isPub;
	vector<repast::AgentId> members;  
} GroupInfo;

class FrameworkModel{
public:	
	FrameworkModel(string propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	~FrameworkModel();

	int stopAt;
	int startX, startY, lengthX, lengthY;
	MPI_Request privateRequest = NULL;
	boost::mpi::communicator *comm;

	MessageInfo privateInfo;
	MessageInfo bcastInfo;

    vector<CommInfo> commInfo;
	map<int, GroupInfo> groupMap;
	int groupSend[MAX_GROUP_CNT];
	int *groupMatrix;

	vector<int> agentTypes;
	repast::Properties* props;
	repast::SharedContext<BaseAgent> context;
	
    repast::SharedDiscreteSpace<BaseAgent, repast::WrapAroundBorders, repast::SimpleAdder<BaseAgent>>* discreteSpace;
	
	int dispatchMessageInfo(MessageInfo *info, int group);
	void messagePoll();
	int initAgents(BaseAgent *agentPtr, string agentPropsFile);
	void runStep();
	void creatGroup();


	void runModel();
	void initSchedule(repast::ScheduleRunner& runner);
};

#endif
