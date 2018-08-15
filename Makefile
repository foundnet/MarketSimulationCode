include ./env

SRCS = $(wildcard ./src/*.cpp)  
OBJS = $(patsubst %.cpp, %.o, $(SRCS))  
TARGET = ./bin/MktSimuApp.exe


all: $(TARGET)
	@echo -----Compile work finished-----


.PHONY: clean
	@rm -f ./bin/*
	@rm -f ./*.o

$(OBJS) : %.o : %.cpp
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c $(SRCS)

$(TARGET) : $(OBJS)	
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o $(TARGET) ./*.o $(REPAST_HPC_LIB) $(BOOST_LIBS)
	@rm -f ./*.o