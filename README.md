# pg-aisd-2025

My project solutions for the 2025 Algorithms & Data Structures class at PG (Gdańsk University of Technology).

All three solutions received maximum points possible.

## Short description & notes

For all three projects, the use of the C++ standard library was prohibited (with a minor exception in project 1 for `std::move`, `std::swap`, and `std::forward`, after asking for it).

Each project subdirectory contains a `tests` subdirectory containing test inputs and the expected outputs (except for project 1, I seem to have not kept the expected outputs).

### Project 1

The project was to implement an interpreter for a simple linked-list-based stack machine (a Lithpm if you will).
The main challenge, and the reason for the code being so weird, is the restriction that no arrays (except for storing the input), and no loops (except for one, but none were used here) could be used.

Known bugs:

 - `ll_item::numerically_lesser` is broken when comparing numbers of different lengths, but none of the tests caught this.
   As a side effect, the logic in `ll_item::add` around adding a negative and a positive number together is slightly wonky as well.
 
### Project 2

The project was to implement a shortest path search through the given given a height map, a list of ski lifts, and fixed rules on how much time moving between position takes.

The algorithm used here is just Dijkstra's algorithm with a priority queue.
I have used a slightly unconventional data structure for the priority queue, a pairing heap, though, while I assume the expected implementation (also probably the optimal one) was a simple binary heap.

One of the challenges was speed optimization, as the later tests were huge (2000x2000 height maps, path to find between opposite corners), and some later test cases involving ski lifts were constructed in such a way to penalize linear searches through the list of lifts.

### Project 3

The project was to implement an integer-keyed trie.
This one was really simple (I completed it the same day it was posted), although it required some memory optimization, as the test cases had very low memory limits.

This is why the trie nodes do not store their size, and it is instead derived from the position in the trie (root/non-root), as storing the size as a member of the node blew past the memory limit.
Another optimization was lazily allocating the array of children in each node, which was also needed to fit in the memory limits (although the implementation here is not as lazy as it could be).
