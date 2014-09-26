# Heat Map with OpenMP

Implement a program that simulates heat distribution on a two-dimensional field. The simulation is executed in rounds. The field is divided into equal-sized blocks. Initially some of the blocks are cold (value=0), some other blocks are active hot spots (value=1). The heat from the hot spots then transfers to the neighbor blocks in each of the rounds, which changes their temperature value.

A round is computed as follows:

1. The value of the hot spot fields may be set to 1 again, depending on the live time of the hot spot during a given number of rounds.
2. The new value for each block per round is computed by getting the values of the eight direct neighbor blocks from the last round. The new block value is the average of these values and the own block value from the last round. Blocks on the edges of the field have neighbor blocks outside of the fields, which should be considered to have the value 0.

You have to develop a parallel application for this simulation in C / C++ or Fortran, which only uses OpenMP. The goal is to minimize the execution time of the complete simulation. Specific optimizations for the given test machine (such as a fixed number of pinned threads) are not allowed, since we may have to opportunity to run your code on some larger computer cluster for the performance comparison.

## Input

Your application has to be named “heatmap” and needs to accept five parameters:

- The width of the field in number of blocks.
- The height of the field in number of blocks.
- The number of rounds to be simulated.
- The name of a file (in the same directory) describing the hotspots.
- The name of a file (in the same directory) containing coordinates. This is the only optional
parameter. If it is passed, only the values at the indicated coordinates (starting at (0, 0) in the upper left corner) are to be written to the output file.

Example:
```
./heatmap 20 7 17 hotspots.csv
./heatmap 20 7 17 hotspots.csv coords.csv
```

The hotspots file has the following structure:

- The first line can be ignored.
- All following lines describe one hotspot per line. The first two values indicate the position in
the heat field (x, y). The hot spot is active from a start round (inclusive), which is indicated by the third value, to an end round (exclusive!), that is indicated by the last value of the line.

Example content of hotspots.csv:
```
x,y,startround,endround
5,2,0,20
15,5,5,15
```

With such an input file, you have to run a simulation of 17 rounds on a 20x7 field with two hotspots. The first one will be located at the coordinates (5, 2) and will be active from the first round to last round (and beyond). The second hotspot will be located at the coordinates (15, 5) and will be active starting from round 5. Starting from round 15, it will no longer be active. The starting round is inclusive, the final round is exclusive. We start counting at 0. So the first hotspot will be active at round 0,1,2...18,19; the second hotspot will be active at round 5,6,7,...13,14.

Example content of coords.csv:
```
x,y
5,2
10,5
```

With such a coordinate file, you only have to provide the values at the coordinates (5, 2) and (10, 5) as part of the output file.

## Output

The program must terminate with exit code 0 and has to produce an output file with the name “output.txt” in the same directory.

If your program was called without a coordinate file, then this file represents the resulting field after simulation termination. The values in the field are encoded in the following way:

- A block with a value larger than 0.9 has to be represented as “X”.
- All other values must be increased by 0.09. From the resulting value, the first digit after the
decimal point is added to the output picture.

Example content of “output.txt” without coordinate file:
```
11112221111111111100
11123432111111111110
11124X42211111111111
11124442111111222111
11122222111112222211
11111211111112232211
01111111111111222111
```

If your program was called with a coordinate file, then this file simply represents the list of exact values requested through the coordinate file.

Example content of “output.txt” with coordinate file:
```
1.0
0.03056341073335933
```
