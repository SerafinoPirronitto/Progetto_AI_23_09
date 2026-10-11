# Compiling
If you use GCC to compile the program, just modify `CMakePresets.json`'s compiler path to yours and run the command `cmake --build --preset GCC-build`. If you use another compiler you might have to do more work than that and it's up to you. The important thing is that **YOU MUST USE** c++23 standard (check your compiler version requirements for this) and utf-8 encoding, which should be the default for c++23 anyway, but some compilers don't do this.

# General info on this code
All the important, theoretical stuff is located in the `AntColony.hpp` header and its implementation `AntColony.cpp`. In these files I made extended use of greek letters such as  "α" or "β", eventhough this is not common in source code, it helps reading the code like the standard mathematical definition of the algorithm a little better than the english phonetic translations such as "alpha" or "beta".

All the other files contains utils for the code and can therefore be ignored, with, maybe, the exception of `Solution.hpp` which contains a class for representing solutions in memory and could be read for better understanding of the code.

In particular:
- `Matrix.hpp` contains a class that represents a matrix like object, useful for everything spanning from adjacency matrixes to pheromones tables.
- `Parser.hpp` contains helper functions for parsing an input file and extrancing the data as matrixes.
- `main.cpp` contains the main loop which executes 5 runs, with different values of each parameter and prints to the standard output the results of each run. Could be a little verbose, so I suggest piping the output to a log.txt file and using grep or any text editor's search functionality to inspect the output. (On windows powershell some work is required in order to not force the output file to be encoded as utf-16, but rather to utf-8, which is reccomended)
