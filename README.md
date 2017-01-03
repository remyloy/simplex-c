# simplex-c

Baseline implementation of the downhill simplex algorithm in C

This implementation is used as a baseline for comparisons against the F# implementation.
The C implementation is not as generic as the F# implementation, as it only works
with a fixed number of parameters and a fixed simulated function.

Compiled with Release/x64 and run on an Intel Core i7-6600U it takes on average 4ms to
calculate 50 iterations.