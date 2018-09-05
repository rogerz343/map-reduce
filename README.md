#map-reduce

A basic distributed computing system partially inspired by MapReduce.

Overview of how it works:
- User adds the desired `map_func` and `reduce_func` functions to *maptask_template.cpp* and *reducetask_template.cpp*. Those two compile to the executables *maptask* and *reducetask*, which every worker should have before startup.

`map` function has the signature `(string, string) -> (string, string)`.

`reduce` function has the signature `(string, list(string)) -> string`.

It is up to the user to determine what each `string` represents and to parse them. For example, the final result (the output of `reduce`) could be a list.

##TODO
- add more error checking and failure handling
- use (smart) pointers when copying in vectors so you don't copy by value
- general make code more efficient
- change wrap std::cout in a function that's only called when debug mode is on