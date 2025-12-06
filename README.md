# CS1412 – Programming Principles | Chord Analyzer (polynizer)
**Authors:** Josue Rodriguez & Arianna Saborío · **Instructor:** Dr. Arturo Camacho

This program analyzes piano-roll data and outputs chord suggestions for each time window, following the rules described in the project handout. It includes the **extra-credit trie-based dictionary lookup**, implemented in `src/trie.c` and used in `src/dictADT.c` to perform longest-prefix matching on the duodecimal word generated from pitch-class centroids. The project structure is: `src/` (source code), `dict/` (dictionary), `data/` (input files), and `out/` (generated outputs).

To build the program, run `make` from the project root. This compiles all files and produces the executable `polynizer`. To clean, run `make clean`.

To run the analyzer, provide four arguments: the `.dat` volume file, the `.begin.txt` file, the `.end.txt` file, and the chord dictionary. Example:

./polynizer "data/Daft Punk - Get lucky.dat" "data/Daft Punk - Get lucky (intro).begin.txt" "data/Daft Punk - Get lucky (intro).end.txt" dict/ChordsDictBasic.txt > "out/Daft Punk - Get lucky (intro).out.txt"


The output lists the three input files first, followed by blocks of:

<index>: <start time>
<chord 1>
<chord 2> (optional)
<chord 3> (optional)

with a blank line between blocks.

Internally, the program folds each 88-key row into 12 pitch classes, applies the 3.5% threshold, computes centroids, constructs the duodecimal word, and queries the trie for chord interpretations. Sharp/flat naming is chosen once based on the initial tonic and kept consistent throughout. All code and logic follow the specifications required for the assignment and the trie implementation fulfills the extra-credit requirement.

Link to GitHub: https://github.com/sabsUG/CS1412-ChordAnalyzer
