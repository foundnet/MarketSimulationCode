include ./env


all: simu
	@echo -----Compile work finished-----


.PHONY: clean
	rm -f ./bin/exe/*
	rm -f ./bin/obj/*

simu: ./src/*.cpp ./include/*
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/*.cpp -o ./bin/obj/*.o
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/exe/MktSimuApp.exe  ./bin/obj/*.o $(REPAST_HPC_LIB) $(BOOST_LIBS)

