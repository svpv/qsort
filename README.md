# qsort.h - Quicksort as a C macro

This is a traditional [Quicksort](https://en.wikipedia.org/wiki/Quicksort)
implementation which for the most part follows
[Robert Sedgewick's 1978 paper](http://penguin.ewu.edu/cscd300/Topic/AdvSorting/Sedgewick.pdf).
It is implemented as a C macro, which means that comparisons can be inlined.
A distinctive feature of this implementation is that it works entirely on array
indices, while actual access to the array elements is abstracted out with
the `less` and `swap` primitives provided by the caller.  Here is an example
of how to sort an array of integers:

```c
#include "qsort.h"
void isort(int A[], size_t n)
{
    int tmp;
#define LESS(i, j) A[i] < A[j]
#define SWAP(i, j) tmp = A[i], A[i] = A[j], A[j] = tmp
    QSORT(n, LESS, SWAP);
}
```
Since access to the actual array is so completely abstracted out,
the macro can be used to sort a few dependent arrays (which,
to the best of my knowledge, no other implementation can do):

```c
#include "qsort.h"
int sortByAge(size_t n, const char *names[], int ages[])
{
    const char *tmpName;
    int tmpAge;
#define LESS(i, j) ages[i] < ages[j]
#define SWAP(i, j) tmpName  = names[i], tmpAge  = ages[i], \
                   names[i] = names[j], ages[i] = ages[j], \
                   names[j] = tmpName,  ages[j] = tmpAge
    QSORT(n, LESS, SWAP);
}
```
The sort is not [stable](https://en.wikipedia.org/wiki/Sorting_algorithm#Stability)
(this is inherent to most of Quicksort variants).  To impose order among
the names with the same age, the `LESS` macro can be enhanced like this:

```c
#define LESS(i, j) ages[i] <  ages[j] || \
                  (ages[i] == ages[j] && strcmp(names[i], names[j]) < 0)
```
<sub>This Quicksort implementation is written by Alexey Tourbin.
The source code is provided under the
[MIT License](https://en.wikipedia.org/wiki/MIT_License).</sub>

## Performance

A [benchmark](bench.cc) is provided which evaluates the performance
of a few implementations: libc's `qsort(3)`, STL's `std::sort` (denoted
resp. `stdlib` and `stl`), Michael Tokarev's
[Inline QSORT() implementation](http://www.corpit.ru/mjt/qsort.html),
and this implementation (denoted resp. `mjt` and `svpv`).
Michael Tokarev's implementation is based on an older glibc's version
of Quicksort.  Modern glibc versions, including the one used below,
use [merge sort](https://en.wikipedia.org/wiki/Merge_sort).

A word of warning: this benchmark does only a tiny bit of averaging.
For conclusive evidence, the program needs to be run multiple times.

By default, the `bench` program sorts 1M random integers.

```
$ make
g++ -O2 -g -Wall -fwhole-program -o bench bench.cc
./bench
stdlib     402584990    19644762
stl        230878632    25344013
mjt        272302466    24316349
svpv       245908342    23287211
```
The STL implementation turns out to be the fastest (the first column
indicates the number of
[RDTSC cycles](https://en.wikipedia.org/wiki/Time_Stamp_Counter)),
despite the fact that it performs the largest number of comparisons
(the second column).

One reason my implementation comes in second to STL is some if its design
limitations.  The `swap` macro issues three moves as a whole, while some
parts of the algorithm, notably
[insertion sort](https://en.wikipedia.org/wiki/Insertion_sort),
can benefit from copying items to the right one position rather than doing
full exchanges.  (It wouldn't be enough to factor `swap` into `save`,
`restore`, and `copy`, though.  After an item is saved to the temporary
register, it is further required to compare other items to that temporary
register, which `less` can't do.)

Of course, to a considerable degree, performance depends on the compiler
being used.  I found that my implementation is favoured by Clang, which
also disrespects `std::sort` (using `-O3` doesn't help).  Sedgewick was
right when he said we exposed ourselves to the whims of compilers.

```
$ rm -f bench && make CXX=clang
clang -O2 -g -Wall -fwhole-program -o bench bench.cc
clang: warning: optimization flag '-fwhole-program' is not supported
./bench
stdlib     414620784    19644762
stl        321896126    25344013
mjt        270644434    24316349
svpv       233669286    23287211
```

Relevant to performance is another characteristic of my implementation:
it does not assume that comparisons are cheap, as with integers, and
deliberately tries to reduce the number of comparisons when it is easily
possible (specifically, during insertion sort, it does not trade boundary
checks for extra comparisons).  This pays off when comparisons are
expensive, such as when comparing string keys with `strcmp(3)`.
In the following example, I use filenames and dependencies from the RPM
database as the set of strings to be sorted, shuffling them with `shuf(1)`.

```
$ rpm -qa --list --requires --provides | shuf >lines
$ wc -l <lines
396681
$ ./bench --strcmp <lines
stdlib     551992820    6883350
stl        607697244    8705273
mjt        572559714    8442262
svpv       535972516    8127244
```
My implementation comes in first now, and `std::sort` falls behind by
a big margin!  Note that `qsort(3)` makes even fewer comparisons (this is
inherent to merge sort as opposed to any Quicksort variant), which makes
it come in second.  Again, Clang favours my implementation even more.

<sub>In the above examples, GCC 6.3.1 and Clang 3.8.0 have been used
on a Haswell CPU.</sub>
