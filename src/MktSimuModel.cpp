/* Demo_03_Model.cpp */

#include <stdio.h>
#include <vector>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/Point.h"

#include "MktSimuModel.h"

using namespace std;


MktSimuModel::MktSimuModel(string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm){
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	startX = repast::strToInt(props->getProperty("startX.of.map"));
	startY = repast::strToInt(props->getProperty("startY.of.map"));
	lengthX = repast::strToInt(props->getProperty("lengthX.of.map"));
	lengthY = repast::strToInt(props->getProperty("lengthY.of.map"));

	initializeRandom(*props, comm);
	
    repast::Point<double> origin(startX,startY);
    repast::Point<double> extent(lengthX, lengthY);
    
    repast::GridDimensions gd(origin, extent);
    
    std::vector<int> processDims;
    processDims.push_back(2);
    processDims.push_back(2);
    
    discreteSpace = new repast::SharedDiscreteSpace<BaseAgent, repast::WrapAroundBorders, repast::SimpleAdder<BaseAgent> >("AgentDiscreteSpace", gd, processDims, 2, comm);
	
    std::cout << "RANK " << repast::RepastProcess::instance()->rank() << " BOUNDS: " << discreteSpace->bounds().origin() << " " << discreteSpace->bounds().extents() << std::endl;
    
   	context.addProjection(discreteSpace);
	
}

MktSimuModel::~MktSimuModel(){
	delete props;

}

int MktSimuModel::initAgents(BaseAgent *agentPtr, string agentPropsFile){

    repast::Properties* agentProps = new repast::Properties(agentPropsFile, comm);

    if (agentProps == NULL)     return 0;

	int agentType = repast::strToInt(props->getProperty("agent.type"));
	int startSeq = repast::strToInt(props->getProperty("start.sequence"));
	int endSeq = repast::strToInt(props->getProperty("end.sequence"));
	int group = repast::strToInt(props->getProperty("agent.group"));
	int posX = repast::strToInt(props->getProperty("position.X"));
	int posY = repast::strToInt(props->getProperty("position.Y"));
    
	int rank = repast::RepastProcess::instance()->rank();

	for(int i = startSeq; i <= endSeq; i++){
        if (posX + posY <= 0)
        {
            posX = (int)(repast::Random::instance()->nextDouble()*lengthX);
            posY = (int)(repast::Random::instance()->nextDouble()*lengthY);
        } 
        repast::Point<int> initialLocation((int)discreteSpace->dimensions().origin().getX() + posX,(int)discreteSpace->dimensions().origin().getY() + posY);

		repast::AgentId id(i, rank, agentType);
		id.currentRank(rank);
		BaseAgent* newAgent = agentPtr->clone(id, agentProps);
        newAgent->init();
		context.addAgent(newAgent);
        discreteSpace->moveTo(id, initialLocation);
	}
}


void MktSimuModel::runStep(){
	int whichRank = 0;

	vector<BaseAgent*> agents;
	vector<int>::iterator it;
	for(it=vec.begin();it!=vec.end();it++)	{
		context.selectAgents(repast::SharedContext<BaseAgent>::LOCAL, agents);
		vector<BaseAgent*>::iterator it = agents.begin();
		while(it != agents.end())
        {
		    (*it)->runStep();
			it++;
		}
	}
}

void MktSimuModel::initSchedule(repast::ScheduleRunner& runner){
	//Run the agents' process functions
    runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<MktSimuModel> (this, &MktSimuModel::runStep)));

    //Run communication functions
    runner.scheduleEvent(1, 0.3, repast::Schedule::FunctorPtr(new repast::MethodFunctor<MktSimuModel> (this, &MktSimuModel::MessagePoll)));

	runner.scheduleStop(stopAt);
	
}

int MktSimuModel::DispatchInformation(Information *info)	
{
    if (info->msgHead.senderID != info->msgHead.receiverID)
    {
        BaseAgent *selAgent = context->getAgent(info->msgHead.receiverID);
        if (selAgent != NULL)
            selAgent->msgQueue.pushInfo(info);            
    }
    else
    {
        std::vector<BaseAgent*> agents;
        if (info->msgHead.msgType !=0 )
            context.selectAgents(agents, info->msgHead.msgType);
        else
            context.selectAgents(agents);

        vector<BaseAgent*>::iterator it = agents.begin();
	    while(it != agents.end())   
        {
    		(*it)->msgQueue.pushInfo(info);
		    it++;
	    }
    }
    return 0;
}


int MktSimuModel::MessagePoll()
{
    int flag;
    MPI_Status status;

    if (privateRequest == NULL)
        MPI_Irecv(&privateInfo, MAX_MSG_LEN, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &privateRequest);

    MPI_Test(&privateRequest, &flag, &status);

    while (flag)
    {
        DispatchInformation(&privateInfo);
        memset(&privateInfo, sizeof(privateInfo), 0 );
        MPI_Irecv(&privateInfo, MAX_MSG_LEN, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &privateRequest);
        MPI_Test(&privateRequest, &flag, &status);
    }

    return 1;
}
