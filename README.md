A basic distributed computing system partially inspired by MapReduce.

Overview of how it works:


`map` function has the signature `(string, string) -> (string, string)`.

`reduce` function has the signature `(string, list(string)) -> string`.

It is up to the user to determine what each `string` represents and to parse them. For example, the final result (the output of `reduce`) could be a list.

Map tasks have the following properties:
- the input file is called "[taskname].mtin"
- the output file is called "[taskname].mtout"
- the executable file is called "[taskname].mtrun"
- all of the above files are in the same directory (which is also the directory that the run_worker exectuable is in)
Reduce tasks have the following properties:
- the input file is called "[taskname].rtin"
- the output file is called "[taskname].rtout"
- the executable file is called "[taskname].rtrun"
- all of the above files are in the same directory (which is also the directory that the run_worker exectuable is in)

Differences from MapReduce (and other notes)
- All input and output key/values are stored in memory and are sent to different machines as bytes (text-encoding) through a network, i.e. there is no shared filesystem.
- The function signatures for map and reduce might be slightly different from other implementations.