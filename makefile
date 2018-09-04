test: test_master run_worker

test_master: master.o machine.o definitions.h test_master.cpp
	g++ -std=c++14 -Wall -o test_master test_master.cpp ./master.o ./machine.o

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
	rm -f test_master

	# can't use "rm -f" because too many files to remove
	# find ./map_in_splits/ -name "*" -print0 | xargs -0 rm
	touch map_in_splits/placeholder.txt
	# find ./map_out/ -name "*" -print0 | xargs -0 rm
	touch map_out/placeholder.txt
	# find ./red_out/ -name "*" -print0 | xargs -0 rm
	touch red_out/placeholder.txt