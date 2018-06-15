#--------------------------------------------------------------

OBJ = logger.o helper.o testvm.o controller.o
CC = g++
FLAGS = -std=c++11 -U__STRICT_ANSI__

#--------------------------------------------------------------


all: controller.exe

logger.o : ../common/header/logger.h
	$(CC) $(FLAGS) -c ../common/logger.cpp

helper.o : header/helper.h 
	$(CC) $(FLAGS) -c helper.cpp 

testvm.o : header/testvm.h ../common/header/command.h
	$(CC) $(FLAGS) -c testvm.cpp

controller.o : header/controller.h
	$(CC) $(FLAGS) -c controller.cpp

controller.exe : $(OBJ)
	$(CC) $(FLAGS) -o controller.exe $(OBJ)


.PHONY: clean cleanall

clean:
	rm -f *.o

cleanall:
	rm -f *.o
	rm -f controller.exe
