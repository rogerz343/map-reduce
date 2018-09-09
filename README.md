# map-reduce

A basic distributed computing system partially inspired by MapReduce.

## Brief descriptions of files
*./maptask_template.cpp*: The source code that is compiled into an executable called "maptask". The user should fill in the `map_func` function to perform the desired task.
*./reducetask_template.cpp*: The source code that is compiled into an executable called "reducetask". The user should fill in the `reduce_func` function to perform the desired task.
*./master.h*: Defines the `Master` class/object, which acts as the server/master in the MapReduce setup. It sends tasks to `Worker`s that connect to it.
*./worker.h*: Defines the `Worker` class/object, which acts as a client/worker in the MapReduce setup. The `Worker` receives tasks from the `Master` and executes the *./maptask* and *./reducetask* executables on the tasks given by the `Master`.
*./run_master*: creates a `Master` instance and starts the server.
*./run_worker*: creates a `Worker` instance that connects to the server.

## Required directories
The following directories must exist with the exact names:
- ./map_in_splits
- ./map_out
- ./intermediate_out
- ./red_out

## maptask and reducetask executables
`map` function has the signature `(string, string) -> (string, string)`.

`reduce` function has the signature `(string, list(string)) -> string`.

It is up to the user to determine what each `string` represents and to parse them. For example, the final result (the output of `reduce`) could be a list.

##TODO
- add more error checking and failure handling
- use (smart) pointers when copying in vectors so you don't copy by value
- general make code more efficient
- change wrap std::cout in a function that's only called when debug mode is on