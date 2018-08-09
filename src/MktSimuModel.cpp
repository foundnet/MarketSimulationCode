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




MktSimuModel::MktSimuModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm){
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	int startX = repast::strToInt(props->getProperty("startX.of.map"));
	int startY = repast::strToInt(props->getProperty("startY.of.map"));
	int lengthX = repast::strToInt(props->getProperty("lengthX.of.map"));
	int lengthY = repast::strToInt(props->getProperty("lengthY.of.map"));

	initializeRandom(*props, comm);
	if(repast::RepastProcess::instance()->rank() == 0) props->writeToSVFile("./output/record.csv");
	
    repast::Point<double> origin(startX,startY);
    repast::Point<double> extent(lengthX, lengthY);
    
    repast::GridDimensions gd(origin, extent);
    
    std::vector<int> processDims;
    processDims.push_back(2);
    processDims.push_back(2);
    
    discreteSpace = new repast::SharedDiscreteSpace<Agent, repast::WrapAroundBorders, repast::SimpleAdder<Agent> >("AgentDiscreteSpace", gd, processDims, 2, comm);
	
    std::cout << "RANK " << repast::RepastProcess::instance()->rank() << " BOUNDS: " << discreteSpace->bounds().origin() << " " << discreteSpace->bounds().extents() << std::endl;
    
   	context.addProjection(discreteSpace);
	
}

MktSimuModel::~MktSimuModel(){
	delete props;

}

void MktSimuModel::initAgent(Agent *agentPtr, int count, int agentType){
	int rank = repast::RepastProcess::instance()->rank();
	agentTypes.push_back(agentType);

	for(int i = 0; i < count; i++){
        repast::Point<int> initialLocation((int)discreteSpace->dimensions().origin().getX() + (int)(repast::Random::instance()->nextDouble()*lengthX),(int)discreteSpace->dimensions().origin().getY() + (int)(repast::Random::instance()->nextDouble()*lengthY));
		repast::AgentId id(i, rank, agentType);
		id.currentRank(rank);
		Agent* newAgent = agentPtr->clone(id);
		newAgent->init();
		context.addAgent(newAgent);
        discreteSpace->moveTo(id, initialLocation);
	}
}


void MktSimuModel::runStep(){
	int whichRank = 0;
	if(repast::RepastProcess::instance()->rank() == whichRank) std::cout << " TICK " << repast::RepastProcess::instance()->getScheduleRunner().currentTick() << std::endl;

	std::vector<Agent*> agents;
	vector<int>::iterator it;
	for(it=vec.begin();it!=vec.end();it++)	{
		context.selectAgents(repast::SharedContext<RepastHPCDemoAgent>::LOCAL, agents,);
		std::vector<Agent*>::iterator it = agents.begin();
		while(it != agents.end()){
				(*it)->runStep();
				it++;
		}
	}
}

void MktSimuModel::initSchedule(repast::ScheduleRunner& runner){
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<MktSimuModel> (this, &MktSimuModel::runStep)));
	runner.scheduleStop(stopAt);
	
}

void MktSimuModel::recordResults(){
	if(repast::RepastProcess::instance()->rank() == 0){
		props->putProperty("Result","Passed");
		std::vector<std::string> keyOrder;
		keyOrder.push_back("RunNumber");
		keyOrder.push_back("stop.at");
		keyOrder.push_back("Result");
		props->writeToSVFile("./output/results.csv", keyOrder);
    }
}

int DispatchMessage()	{
    Actor *pActor;
    if (repast::RepastProcess::instance()->rank() != 0) // If this is worker node
    {
            pActor = SearchActor(pMsg->rcvActorID);
            if (pActor != NULL)
            {
                sprintf(info, "Dispatch OK actor %d",pActor->actorID);
                LogMsg(info, DEBUG, 0);
 
                int point = (pActor->msgQueue.head + 1) % ACTOR_MSGQUEUE_LEN;
                if (point != pActor->msgQueue.tail)
                {
                    pActor->msgQueue.msg[point] = *pMsg;
                    pActor->msgQueue.head = point;  
                    return 1;
                }
                else return 0;
            }
        }
    }
    return 0;
}

int MessagePoll()
{
    int flag;
    MPI_Status status;

    LogMsg("Framework Poll",DEBUG,0);

    if (cntlRequest == NULL)
        MPI_Irecv(&cntlMessage, 1, AF_CNTL_TYPE, MPI_ANY_SOURCE, AF_CONTROL_TAG, MPI_COMM_WORLD, &cntlRequest);
   
    MPI_Test(&cntlRequest, &flag, &status);
    while (flag)
    {
        LogMsg("Receive a control Msg",DEBUG,0);
        if (!ProcessFrameworkMessage(handleGlobal.rank, &cntlMessage))
            return 0;
        MPI_Irecv(&cntlMessage, 1, AF_CNTL_TYPE, MPI_ANY_SOURCE, AF_CONTROL_TAG, MPI_COMM_WORLD, &cntlRequest);
        MPI_Test(&cntlRequest, &flag, &status);
    }

    if (handleGlobal.rank != 0)
    {
        if (cntlBcastRequest == NULL)
            MPI_Ibcast(&cntlBcastMessage, 1, AF_CNTL_TYPE, 0, MPI_COMM_WORLD, &cntlBcastRequest);

        MPI_Test(&cntlBcastRequest, &flag, &status);
        while (flag)
        {
            char info[100];
            
            LogMsg("Receive a bcast control Msg",DEBUG,0);

            if (!ProcessFrameworkMessage(handleGlobal.rank, &cntlBcastMessage))
                return 0;
            memset(&cntlBcastMessage, 0, sizeof(FrameMessage));

            MPI_Ibcast(&cntlBcastMessage, 1, AF_CNTL_TYPE, 0, MPI_COMM_WORLD, &cntlBcastRequest);
            MPI_Test(&cntlBcastRequest, &flag, &status);
        }
    }

    if (actorRequest == NULL)
        MPI_Irecv(&actorMessage, 1, AF_ACTOR_TYPE, MPI_ANY_SOURCE, AF_ACTORMSG_TAG, 
                    MPI_COMM_WORLD, &actorRequest);
    
    MPI_Test(&actorRequest, &flag, &status);
    while (flag)
    {
        LogMsg("Receive a actor Msg",DEBUG, 0);

        DispatchActorMessage(handleGlobal.rank, &actorMessage);
        MPI_Irecv(&actorMessage, 1, AF_ACTOR_TYPE, MPI_ANY_SOURCE, AF_CONTROL_TAG,
                    MPI_COMM_WORLD, &actorRequest);
        MPI_Test(&actorRequest, &flag, &status);
    }

    return 1;
}
