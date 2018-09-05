
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
    
   	std::cout << "FrameworkModel: stopAt:" << stopAt << " startX:" << startX << " rank:" << repast::RepastProcess::instance()->rank() << std::endl;


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

int FrameworkModel::initAgents(BaseAgent *agentPtr, string agentPropsFile)
{

    repast::Properties *agentProps = new repast::Properties(agentPropsFile, comm);

    if (agentProps == NULL)     return 0;

	int agentType = atoi(agentProps->getProperty("agent.type").c_str());
	int startSeq = atoi(agentProps->getProperty("start.sequence").c_str());
	int endSeq =atoi(agentProps->getProperty("end.sequence").c_str());
	int group = atoi(agentProps->getProperty("agent.group").c_str());
	int posX = atoi(agentProps->getProperty("position.X").c_str());
	int posY = atoi(agentProps->getProperty("position.Y").c_str());
    
	int rank = repast::RepastProcess::instance()->rank();

  	std::cout << "FrameModel:initAgents start:" << startSeq << " end:" << endSeq << " rank:" << repast::RepastProcess::instance()->rank() << std::endl;

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
     	std::cout << "FrameModel:create a agent:" << i << " rank:" << repast::RepastProcess::instance()->rank() << std::endl;
	}

}


void FrameworkModel::runStep()
{
	vector<BaseAgent*> agents;
    context.selectAgents(repast::SharedContext<BaseAgent>::LOCAL, agents);
	vector<BaseAgent*>::iterator it = agents.begin();
  	std::cout << "FrameModel:runStep tick:" << repast::RepastProcess::instance()->getScheduleRunner().currentTick() << " rank:" << repast::RepastProcess::instance()->rank() << std::endl;

	while(it != agents.end())
    {
        (*it)->runStep();
		it++;
	}
}

void FrameworkModel::initSchedule(repast::ScheduleRunner& runner){
	//Run the agents' process functions
    runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<FrameworkModel> (this, &FrameworkModel::runStep)));
    runner.scheduleEvent(1, 0.3, repast::Schedule::FunctorPtr(new repast::MethodFunctor<FrameworkModel> (this, &FrameworkModel::messagePoll)));

    //Run communication functions

	runner.scheduleStop(stopAt);
	
}

int FrameworkModel::dispatchMessageInfo(MessageInfo *info, int groupNum)	
{
    if (groupNum < 0)           //Private
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
    }
    else                        //Broadcast 
    {
        for (int i=0; i<groupMap[groupNum].members.size() ;i++)
        {
            BaseAgent *agent = context.getAgent(groupMap[groupNum].members[i]);
            agent->msgQueue.pushInfo(info);
        }
    }
    return 1;
}


void FrameworkModel::messagePoll()
{
    int flag;
    MPI_Status status;
  	std::cout << "FrameModel:MesagePoll tick:" << repast::RepastProcess::instance()->getScheduleRunner().currentTick() << " rank:" << repast::RepastProcess::instance()->rank() << std::endl;

    if (privateRequest == NULL)
        MPI_Irecv(&privateInfo, MAX_MSG_LEN, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &privateRequest);

    MPI_Test(&privateRequest, &flag, &status);

    while (flag)
    {
        dispatchMessageInfo(&privateInfo, -1);
        memset(&privateInfo, sizeof(privateInfo), 0 );
        MPI_Irecv(&privateInfo, MAX_MSG_LEN, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &privateRequest);
        MPI_Test(&privateRequest, &flag, &status);
    }
    
    for (int i=0; i<commInfo.size(); i++)
    {
        if (commInfo[i].pubRank != repast::RepastProcess::instance()->rank())
        {
            if (commInfo[i].request == NULL)
                MPI_Ibcast(&bcastInfo, MAX_MSG_LEN, MPI_UNSIGNED_CHAR, commInfo[i].pubRank, commInfo[i].comm, &commInfo[i].request);
            MPI_Test(&commInfo[i].request, &flag, &status);

            while (flag)
            {
                dispatchMessageInfo(&bcastInfo, commInfo[i].groupNum);
                memset(&bcastInfo, 0, sizeof(MessageInfo));

                MPI_Ibcast(&bcastInfo, MAX_MSG_LEN, MPI_UNSIGNED_CHAR, commInfo[i].pubRank, commInfo[i].comm, &commInfo[i].request);
                MPI_Test(&commInfo[i].request, &flag, &status);
            }
        }
    }
}


void FrameworkModel::creatGroup()
{
    boost::mpi::communicator world;
    groupMatrix = new int(world.size() * MAX_GROUP_CNT);
    map<int,GroupInfo>::iterator iter;
    memset(groupSend, 0, MAX_GROUP_CNT);
    for (iter=groupMap.begin(); iter!=groupMap.end(); iter++)
    {
        if (iter->second.isPub)
            groupSend[iter->first] = 2;
        else    groupSend[iter->first] = 1;
    }
    MPI_Allgather(&groupSend, MAX_GROUP_CNT, MPI_INT, groupMatrix, MAX_GROUP_CNT, MPI_INT, MPI_COMM_WORLD);

    MPI_Group MPI_COMM_GROUP;
    MPI_Comm_group(MPI_COMM_WORLD, &MPI_COMM_GROUP);

    int *point;
    int *temp = new int(world.size());
    for (int i=0 ; i<MAX_GROUP_CNT; i++)
    {
        memset(temp, 0, world.size());
        int curPos = 0;
        point = groupMatrix;
        point = point + i;
        int pubNode = -1;
        for (int j=0; j<world.size(); j++)
        {
            if (*point > 0) 
            {
                temp[curPos] = j;
                if (temp[curPos] == 2)  pubNode = j;
                curPos++;
            }
            point = point + MAX_GROUP_CNT;
        }
        if (curPos > 0)
        {
            MPI_Group subGroup;
            CommInfo addComm;
            MPI_Group_incl(MPI_COMM_GROUP, curPos, temp, &subGroup);
            MPI_Comm_create(MPI_COMM_WORLD, subGroup, &addComm.comm);
            addComm.groupNum = i;
            addComm.pubRank = pubNode;
            addComm.request = NULL;
            commInfo.push_back(addComm);
        }
    }
   	std::cout << "FrameModel:creatGroup count:" << commInfo.size() << " rank:" << repast::RepastProcess::instance()->rank() << std::endl;

    delete temp;
}


void FrameworkModel::runModel()
{
    creatGroup();
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    initSchedule(runner);
    runner.run();
}
