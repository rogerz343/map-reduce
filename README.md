Program inspired by MapReduce.

`map` function has the signature `(string, string) -> (string, string)`.

`reduce` function has the signature `(string, list(string)) -> string`.

It is up to the user to determine what each `string` represents and to parse them. For example, the final result (the output of `reduce`) could be a list.

Differences from MapReduce (and other notes)
- All input and output key/values are stored in memory and are sent to different machines as bytes (text-encoding) through a network, i.e. there is no shared filesystem.
- The function signatures for map and reduce might be slightly different from other implementations.