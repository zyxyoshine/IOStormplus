#--------------------------------------------------------------

OBJ = agent.o baseagent.o logger.o
CC = g++
FLAGS = -std=c++11

#--------------------------------------------------------------


all: agent

logger.o : ../../common/header/logger.h
	$(CC) $(FLAGS) -c ../../common/logger.cpp

baseagent.o : ../header/baseagent.h
	$(CC) $(FLAGS) -c ../baseagent.cpp 

agent.o : header/agent.h
	$(CC) $(FLAGS) -c agent.cpp

agent : $(OBJ)
	$(CC) $(FLAGS) -o agent $(OBJ)


.PHONY: clean cleanall

clean:
	rm -f *.o

cleanall:
	rm -f *.o
	rm -f agent
