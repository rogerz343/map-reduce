master.o: master.h master.cpp
	g++ -g -Wall -c -std=c++14 master.cpp

clean:
	rm master.o