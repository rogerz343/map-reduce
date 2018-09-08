test: run_master run_worker maptask reducetask

run_master: run_master.cpp master.o definitions.h
	g++ -std=c++14 -Wall -o run_master run_master.cpp ./master.o

master.o: master.h master.cpp definitions.h 
	g++ -std=c++14 -Wall -c master.cpp

run_worker: run_worker.cpp worker.o definitions.h
	g++ -std=c++14 -Wall -o run_worker run_worker.cpp ./worker.o

worker.o: worker.h worker.cpp definitions.h
	g++ -std=c++14 -Wall -c worker.cpp

maptask: maptask_template.cpp definitions.h
	g++ -std=c++14 -Wall -o maptask maptask_template.cpp

reducetask: reducetask_template.cpp definitions.h
	g++ -std=c++14 -Wall -o reducetask reducetask_template.cpp

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