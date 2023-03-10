=====================
Countdown bruteforcer
=====================

Solver for the challenge posed by Another Roof in his `video about Countdown
<https://youtu.be/X-7Wev90lw4>`_.

The core solver is written in C++, with some helper scripts for parsing the
results written in Python.

Compiling the main solver: ``clang++ find.cpp -o find -O3`` (i found that Clang
optimizes the code a little better than GCC, so I used that, but any C++
compiler should work.)

The solver works by computing look-up tables (or LUTs for short) for 1, 2 or 3
large numbers, and then later finding the success probability of each set of 4
large numbers by combining values from those look-up tables. This combining is
done by the ``combine_lut.py`` script. Situations where 4 large numbers are
chosen are handled by the main solver directly.

The output formats of ``combine_lut.py``, and the main solver when using all 4
large numbers, is like: ``3L:97,69,50,37: 742806/2035371``. The beginning is
one of ``1L``, ``2L``, ``3L``, or ``4L``, specifying how many large numbers
were used (out of the 4 available). Then follow the 4 large numbers themselves.
Then comes the total number of winnable games: this is 2 numbers separated by a
``/``, where the first one is actually different winnable games, and the second
one is winnable games weighted by the probability of them occurring. These are
different because, for example, when choosing 4 large numbers, scenarios where
the last 2 small numbers are the same are 4x less likely to actually happen
than scenarios where the last 2 are different.

Approach
========

Look-up tables
--------------

The LUTs for 1 large, 2 large, or 3 large chosen numbers work as follows. For
1LUT, a line like "1LUT:82: 1263657/6882261" means that out of all games with
gamesets containing only 1 large number, that number being 82, there were
1263657 winnable games (or 6882261 weighted winnable games). Then, to compute
the number of winnable games of a set of 4 large numbers, it suffices to add the
corresponding 1LUT entries.

For 2LUT, the format is similar: "2LUT:36,24: 535047/4287720" means there were
535047 winnable games with gamesets containing the large numbers 36 and 24, and
4 small numbers. To compute the answer for a set of 4 large numbers then, it is
necessary to add the 2LUT entries corresponding to all pairs of the 4 large
numbers: for a set like (82, 61, 36, 24) it would be the entries for the pairs
(82, 61), (82, 36), (82, 24), (61, 36), (61, 24), (36, 24).

3LUT is the same as 2LUT except with 3 numbers instead. For 4 large numbers,
there is no LUT, as the LUT would be just as big as the results table itself.

Solving a single gameset
------------------------

For a single gameset of 6 numbers, the solver works as follows. For each pair of
numbers in the input, it combines them using the 4 possible operations, and in
case of a valid result, recursively calls itself again, removing the 2 operands
from the set and adding the result. The solver returns these results as a bitset
of 900 bits, each bit representing whether it is possible to reach a number from
100 to 999 using the given set. The results from these recursive calls are then
OR'd together to get all reachable target numbers.

One important aspect of this solver is memoization: there is a cache of 4
million or 8 million entires, indexed by the hash of the input set, which gets
filled each time the recursive solver finds a result for one set. The solver
also checks at the beginning of each recursive call whether the input already
exists in the cache, and if it does, returns that instead. If there is a hash
collision, the new value just overwrites the old one.

In the end, inside solving a single gameset, this kind of cache has a hit rate
of around 90%, which corresponds to even more than 90% time saved, since some of
those hits were at high levels of recursion, and allowed pruning a large branch
of the search tree with each hit.

Also, this means that when solving related gamesets in sequence, the new gameset
can use all cached values from the previous one, which gives an even further
speedup, as in most cases inputs to consecutive solver calls will only differ in
1 or 2 numbers.

Running the solver
==================

The main solver can be invoked in 4 different ways, corresponding to computing
the LUTs for 1, 2, or 3 chosen large numbers, and for computing the results
directly for 4 chosen larges. The most time-consuming part is computing the 4
larges results: this took around 45 CPU-hours in total on my laptop (but i have
6 cores, so it only took around 8 hours of real time in total). Computing the
LUTs for 1 and 2 chosen larges takes pretty much negligible amounts of time,
but the LUT for 3 chosen larges takes another 7 CPU-hours.

Note that all the filenames in this section are arbitrary, you don't have to
use these exact names. Also, for the 3LUT/4-large sections, if you parallelize
it then you might want to use a different output name for each job, to prevent
one job from overwriting another job's outputs.

To compute the 1-chosen LUT, simply do ``./find lut1 >lut1.txt``.

To compute the 2-chosen LUT, you need to do ``./find lut2 12 >>lut2.txt`` all
the way up to ``./find lut2 100 >>lut2.txt``. I used a Bash script to automate
this and run it on a few cores simultaneously (e.g. let one Bash script do all
the odd numbers and the other one all the even numbers).

To compute the 3-chosen LUT, you need to run ``./find lut3 100 99 >>lut3.txt``
all the way down to ``./find lut3 13 12 >>lut3.txt``, over all pairs of numbers
between 12 and 100 with the first one being larger than the second one. Again I
automated this with Bash scripts.

To compute the results for 4 chosen large numbers, run
``./find 4only 100 99 >>results_4only.txt`` down to ``13 12``, just like lut3.

To compute the actual results from the LUTs for 1, 2 or 3 large chosen, run
``python combine_lut.py lut1.txt lut2.txt lut3.txt >results_lut.txt``.

Viewing the outputs
===================

At this point you actually have all of the results. You can use the
``sol_parse.py`` script to find the 10 best/worst sets of large numbers.

To use it, run ``python -i sol_parse.py results_4only.txt results_lut.txt``,
passing in the filenames of all files containing results. Then, call the
function ``show_top10(num_large, max, invert, weighted)`` where 'num_large' is
1, 2, 3, or 4, giving how many large numbers to choose. 'max' is the maximum
possible value, corresponding to every target being winnable with every gameset,
used for computing and showing percentages. You can find appropriate values in
the table below. 'invert' is whether to reverse the sorting order, i.e. show the
best 10 sets instead of the worst 10. 'weighted' is whether to use the total
number of games or the games weighted by probability of occurrence.

Converting the results to percentages
=====================================

All of the results are computed as "number of winnable games". These aren't
very informative without knowing how many games there are in total though. So
here's a handy chart:

+---------------+-------------+-------------------------------------+
| Larges chosen | Total games | Weighted total games                |
+===============+=============+=====================================+
| 1             | 5227200     | 20C5 * 4C1 * 900 / 2 = 27907200 (*) |
+---------------+-------------+-------------------------------------+
| 2             | 3321000     | 20C4 * 4C2 * 900 = 26163000         |
+---------------+-------------+-------------------------------------+
| 3             | 756000      | 20C3 * 4C3 * 900 / 2 = 2052000 (*)  |
+---------------+-------------+-------------------------------------+
| 4             | 49500       | 20C2 * 4C4 * 900 = 171000           |
+---------------+-------------+-------------------------------------+

The "Total games" column was computed by Alex in the original video.

The weighted column is just number of options for small numbers, times number
of options for large numbers, times 900 for the number of possible targets.

The values marked with (*) actually should be 2x larger, however in the solver I
normalized the values so that the "rarest" sets all have a weight of 1 (for
3large these are the sets with 1 pair, for 1large it's sets with 2 pairs). So
the values in the table are what you will actually see in the results and what
you can use to compute percentages. If you wanted the real total games, you
should multiply all results from 1L or 3L by 2.

Summary of results
==================

Here are the best sets for each number of chosen large numbers:

+---------------+----------------+---------------------+-----------------+-----------------------+
| Larges chosen | Best (unique)  | # winnable (unique) | Best (weighted) | # winnable (weighted) |
+===============+================+=====================+=================+=======================+
| 1             | 97, 95, 93, 83 | 5078791 (97.16%)    | 97, 95, 94, 93  | 27599821 (98.90%)     |
+---------------+----------------+---------------------+-----------------+-----------------------+
| 2             | 97, 75, 62, 13 | 3281410 (98.81%)    | --              | 26046022 (99.55%)     |
+---------------+----------------+---------------------+-----------------+-----------------------+
| 3             | 37, 34, 23, 21 | 754338 (99.78%)     | --              | 2050209 (99.91%)      |
+---------------+----------------+---------------------+-----------------+-----------------------+
| 4             | 57, 17, 14, 12 | 49493 (99.985%)     | --              | 170993 (99.996%)      |
+---------------+----------------+---------------------+-----------------+-----------------------+

A '--' in the weighted column means the best set is the same as the best for unique.

And here are the worst sets:

+---------------+----------------+---------------------+-----------------------+
| Larges chosen | Set            | # winnable (unique) | # winnable (weighted) |
+===============+================+=====================+=======================+
| 1             | 16, 14, 12, 11 | 4686705 (89.66%)    | 26283990 (94.18%)     |
+---------------+----------------+---------------------+-----------------------+
| 2             | 24, 18, 16, 12 | 3091209 (93.08%)    | 25073370 (95.84%)     |
+---------------+----------------+---------------------+-----------------------+
| 3             | 80, 64, 48, 32 | 686495 (90.81%)     | 1909199 (93.04%)      |
+---------------+----------------+---------------------+-----------------------+
| 4             | 98, 97, 49, 48 | 40495 (81.81%)      | 144499 (84.50%)       |
+---------------+----------------+---------------------+-----------------------+

Here, the worst set was always the same, no matter if counted using weighted
games or not.

I'm not sure how to average these together to give a conclusive single answer
to "what is the worst set", as the number of large numbers used isn't uniformly
random but is chosen by the contestants. If we assume that contestants choose
each of the 4 options with equal probability, then the worst set overall is
(80, 64, 48, 32), with a 93.45% chance of being winnable. If we count over all
unique games in total, the worst set is (24, 18, 16, 12) with 8557017 winnable
games out of 9353700 total, or 91.48%.

The raw results are in the zip file ``results_small.zip``: this includes all
results for 4 large numbers chosen, and the 3 LUTs for computing the rest of
the results (which can be combined via the Python script). The full results are
87MB compressed, so I didn't want to put them on GitHub.
