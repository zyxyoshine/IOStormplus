#--------------------------------------------------------------

OBJ = logger.o helper.o testvm.o controller.o
CC = g++
FLAGS = -std=c++11 -U__STRICT_ANSI__

#--------------------------------------------------------------


all: controller.exe

logger.o : ../common/header/logger.h ../common/logger.cpp
	$(CC) $(FLAGS) -c ../common/logger.cpp

helper.o : header/helper.h helper.cpp ../common/header/logger.h ../common/logger.cpp
	$(CC) $(FLAGS) -c helper.cpp 

testvm.o : header/testvm.h testvm.cpp ../common/header/command.h header/helper.h helper.cpp ../common/header/logger.h ../common/logger.cpp
	$(CC) $(FLAGS) -c testvm.cpp

controller.o : header/controller.h controller.cpp header/constant.h header/testvm.h testvm.cpp ../common/header/command.h header/helper.h helper.cpp ../common/header/logger.h ../common/logger.cpp
	$(CC) $(FLAGS) -c controller.cpp

controller.exe : $(OBJ)
	$(CC) $(FLAGS) -o controller.exe $(OBJ)


.PHONY: clean cleanall

clean:
	rm -f *.o

cleanall:
	rm -f *.o
	rm -f controller.exe
