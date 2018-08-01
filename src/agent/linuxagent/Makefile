#--------------------------------------------------------------

OBJ = agent.o baseagent.o logger.o
CC = g++
FLAGS = -std=c++11

#--------------------------------------------------------------


all: agent

logger.o : ../../common/header/logger.h ../../common/logger.cpp
	$(CC) $(FLAGS) -c ../../common/logger.cpp

baseagent.o : ../header/baseagent.h ../baseagent.cpp ../../common/header/command.h ../../common/header/logger.h ../../common/logger.cpp
	$(CC) $(FLAGS) -c ../baseagent.cpp 

agent.o : header/constant.h agent.cpp ../header/baseagent.h ../baseagent.cpp ../../common/header/logger.h ../../common/logger.cpp 
	$(CC) $(FLAGS) -c agent.cpp

agent : $(OBJ)
	$(CC) $(FLAGS) -o agent $(OBJ) -lboost_system -lcrypto -lssl -lcpprest -lazurestorage


.PHONY: clean cleanall

clean:
	rm -f *.o

cleanall:
	rm -f *.o
	rm -f agent
