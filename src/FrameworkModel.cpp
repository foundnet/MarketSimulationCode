
#include "FrameworkModel.h"

using namespace std;


FrameworkModel::FrameworkModel(string propsFile, int argc, char** argv, boost::mpi::communicator* commWorld): context(commWorld){
	props = new repast::Properties(propsFile, argc, argv, commWorld);
    comm = commWorld;
	stopAt = atoi(props->getProperty("stop.at").c_str());
	startX = atoi(props->getProperty("startX.of.map").c_str());
	startY = atoi(props->getProperty("startY.of.map").c_str());
	lengthX = atoi(props->getProperty("lengthX.of.map").c_str());
	lengthY = atoi(props->getProperty("lengthY.of.map").c_str());

	initializeRandom(*props, comm);
	
    repast::Point<double> origin(startX,startY);
    repast::Point<double> extent(lengthX, lengthY);
    
    repast::GridDimensions gd(origin, extent);
    
    std::vector<int> processDims;
    processDims.push_back(2);
    processDims.push_back(2);
    
    discreteSpace = new repast::SharedDiscreteSpace<BaseAgent, repast::WrapAroundBorders, repast::SimpleAdder<BaseAgent> >("AgentDiscreteSpace", gd, processDims, 2, comm);

   	context.addProjection(discreteSpace);
}

FrameworkModel::~FrameworkModel(){
	delete props;

}

int FrameworkModel::initAgents(BaseAgent *agentPtr, string agentPropsFile){
   	std::cout << "F00000000000 " << repast::RepastProcess::instance()->rank() << std::endl;

    repast::Properties *agentProps = new repast::Properties(agentPropsFile, comm);

    if (agentProps == NULL)     return 0;

   	std::cout << "F111111111111 " << repast::RepastProcess::instance()->rank() << std::endl;

	int agentType = atoi(props->getProperty("agent.type").c_str());
	int startSeq = atoi(props->getProperty("start.sequence").c_str());
	int endSeq =atoi(props->getProperty("end.sequence").c_str());
	int group = atoi(props->getProperty("agent.group").c_str());
	int posX = atoi(props->getProperty("position.X").c_str());
	int posY = atoi(props->getProperty("position.Y").c_str());
    
	int rank = repast::RepastProcess::instance()->rank();
   	std::cout << "F222222222222 " << repast::RepastProcess::instance()->rank() << std::endl;

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
        newAgent->init(&context);
		context.addAgent(newAgent);
        discreteSpace->moveTo(id, initialLocation);
	}
}


void FrameworkModel::runStep()
{
	vector<BaseAgent*> agents;
    context.selectAgents(repast::SharedContext<BaseAgent>::LOCAL, agents);
	vector<BaseAgent*>::iterator it = agents.begin();
	while(it != agents.end())
    {
        (*it)->runStep();
		it++;
	}
}

void FrameworkModel::initSchedule(repast::ScheduleRunner& runner){
	//Run the agents' process functions
    runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<FrameworkModel> (this, &FrameworkModel::runStep)));
    runner.scheduleEvent(1, 0.3, repast::Schedule::FunctorPtr(new repast::MethodFunctor<FrameworkModel> (this, &FrameworkModel::MessagePoll)));

    //Run communication functions

	runner.scheduleStop(stopAt);
	
}

int FrameworkModel::DispatchInformation(Information *info)	
{
    if (info->msgHead.senderID != info->msgHead.receiverID)
    {
        BaseAgent *selAgent = context.getAgent(info->msgHead.receiverID);
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


void FrameworkModel::MessagePoll()
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

}
