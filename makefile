test: run_master run_worker

run_master: master.o machine.o definitions.h run_master.cpp
	g++ -std=c++14 -Wall -o run_master run_master.cpp ./master.o ./machine.o

master.o: master.h master.cpp machine.o definitions.h 
	g++ -std=c++14 -Wall -c master.cpp ./machine.o

run_worker: run_worker.cpp definitions.h 
	g++ -std=c++14 -Wall -o run_worker run_worker.cpp

maptask: maptask_template.cpp definitions.h
	g++ -std=c++14 -Wall -o maptask maptask_template.cpp

reducetask: reducetask_template.cpp definitions.h
	g++ -std=c++14 -Wall -o reducetask reducetask_template.cpp

machine.o: machine.h machine.cpp definitions.h
	g++ -std=c++14 -Wall -c machine.cpp

clean:
	rm -f *.o
	rm -f *.h.gch
	rm -f maptask
	rm -f reducetask
	rm -f run_worker
	rm -f run_master
	rm -rf ./map_in_splits/
	mkdir map_in_splits
	touch map_in_splits/placeholder.txt
	rm -rf ./map_out/
	mkdir map_out
	touch map_out/placeholder.txt
	rm -rf ./red_out
	mkdir red_out
	touch red_out/placeholder.txt
	rm -rf ./intermediate_out
	mkdir intermediate_out
	touch intermediate_out/placeholder.txt

clear_output:
	rm -rf ./map_in_splits/
	mkdir map_in_splits
	touch map_in_splits/placeholder.txt
	rm -rf ./map_out/
	mkdir map_out
	touch map_out/placeholder.txt
	rm -rf ./red_out
	mkdir red_out
	touch red_out/placeholder.txt
	rm -rf ./intermediate_out
	mkdir intermediate_out
	touch intermediate_out/placeholder.txt