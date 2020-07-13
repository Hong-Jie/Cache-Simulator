# Cache-Simulator

In this project, I simulated behavior of cache, where CPU is finding a block to read from or write to. This program provides two modes of picking a block: random and LRU. LRU stands for Least Recently Used, meaning that when all blocks in the cache are occupied, the least recently used block will be taken and used by CPU.  

Four parameters have to be specified while executing the program:
1. _nk_: the capacity of the cache in kilobytes (an int)
2. _assoc_: the associativity of the cache (an int)
3. _blocksize_: the size of a single cache block in bytes (an int)
4. _repl_: the replacement policy (a char); 'l' means LRU, 'r' means random.

After parsing the trace file, this program will output:
1. The total number of misses,
2. The percentage of misses (i.e. total misses divided by total accesses),
3. The total number of read misses,
4. The percentage of read misses (i.e. total read misses divided by total read accesses),
5. The total number of write misses,
6. The percentage of write misses (i.e. total write misses divided by total write accesses).

Sample execution and output:
```
> ./cache 2048 64 64 l
55752 5.575200% 55703 5.610155% 49 0.689752%
```
