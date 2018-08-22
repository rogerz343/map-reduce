test_master: master.o
	g++ -std=c++14 -Wall -o test_master test_master.cpp ./master.o

master.o: master.h master.cpp
	g++ -std=c++14 -Wall -c  master.cpp

run_worker: run_worker.cpp
	g++ -std=c++14 -Wall -o run_worker run_worker.cpp

maptask.o: maptask.h maptask.cpp
	g++ -std=c++14 -Wall -c maptask.cpp

reducetask.o: reducetask.h reducetask.cpp
	g++ -std=c++14 -Wall -c reducetask.cpp

machine.o: machine.h machine.cpp
	g++ -std=c++14 -Wall -c machine.cpp

clean:
	rm -f *.o
	rm -f *.h.gch