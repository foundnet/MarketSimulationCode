

#include "FrameworkModel.h"
#include "CMarketMaker.h"
#include "CMarketParticipant.h"


int main(int argc, char** argv){
	
	std::string configFile = argv[1]; // The name of the configuration file is Arg 1
	std::string propsFile  = argv[2]; // The name of the properties file is Arg 2
	
	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;

	repast::RepastProcess::init(configFile);
	
	std::cout << "0000000000000000 " << repast::RepastProcess::instance()->rank() << std::endl;

	FrameworkModel* model = new FrameworkModel(propsFile, argc, argv, &world);

	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
	
	model->initSchedule(runner);

	MktParticipant factory;

	std::cout << "1111111111111 " << repast::RepastProcess::instance()->rank() << std::endl;
	model->initAgents(&factory, "agent.participant.props");
	std::cout << "2222222222222 " << repast::RepastProcess::instance()->rank() << std::endl;
	
	MarketMaker maker;
	model->initAgents(&maker, "agent.market.props");


	
	delete model;
	
	repast::RepastProcess::instance()->done();
	
}