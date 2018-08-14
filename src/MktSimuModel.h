
#ifndef MKTSIMUMODEL
#define MKTSIMUMODEL

#include <boost/mpi.hpp>
#include "repast_hpc/Schedule.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/AgentRequest.h"
#include "repast_hpc/TDataSource.h"
#include "repast_hpc/SVDataSet.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"

#include "CommonTypes.h"
#include "ObjectClass.h"
#include "CAgent.h"

/* Agent Package Provider */
class RepastHPCDemoAgentPackageProvider {
	
private:
    repast::SharedContext<RepastHPCDemoAgent>* agents;
	
public:
	
    RepastHPCDemoAgentPackageProvider(repast::SharedContext<RepastHPCDemoAgent>* agentPtr);
	
    void providePackage(RepastHPCDemoAgent * agent, std::vector<RepastHPCDemoAgentPackage>& out);
	
    void provideContent(repast::AgentRequest req, std::vector<RepastHPCDemoAgentPackage>& out);
	
};

/* Agent Package Receiver */
class RepastHPCDemoAgentPackageReceiver {
	
private:
    repast::SharedContext<RepastHPCDemoAgent>* agents;
	
public:
	
    RepastHPCDemoAgentPackageReceiver(repast::SharedContext<RepastHPCDemoAgent>* agentPtr);
	
    RepastHPCDemoAgent * createAgent(RepastHPCDemoAgentPackage package);
	
    void updateAgent(RepastHPCDemoAgentPackage package);
	
};


/* Data Collection */
class DataSource_AgentTotals : public repast::TDataSource<int>{
private:
	repast::SharedContext<RepastHPCDemoAgent>* context;

public:
	DataSource_AgentTotals(repast::SharedContext<RepastHPCDemoAgent>* c);
	int getData();
};
	

class DataSource_AgentCTotals : public repast::TDataSource<int>{
private:
	repast::SharedContext<RepastHPCDemoAgent>* context;
	
public:
	DataSource_AgentCTotals(repast::SharedContext<RepastHPCDemoAgent>* c);
	int getData();
};

class MktSimuModel{
	MktSimuModel(string propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	~MktSimuModel();
public:
	int stopAt;
	int startX, startY, lengthX, lengthY;
	MPI_Request privateRequest = NULL;
	MPI_Request bcastRequest = NULL;

	Information privateInfo;
	Information bcastInfo;

	vector<int> agentTypes;
	repast::Properties* props;
	repast::SharedContext<BaseAgent> context;
	
    repast::SharedDiscreteSpace<BaseAgent, repast::WrapAroundBorders, repast::SimpleAdder<BaseAgent>>* discreteSpace;
	
	int DispatchInformation(Information *info);
	int MessagePoll();
	void initAgents(BaseAgent *agentPtr, repast::Properties* agentProps);
	void runStep();
	void initSchedule(repast::ScheduleRunner& runner);
};

#endif
