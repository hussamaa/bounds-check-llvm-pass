### BoundsCheck LLVM Pass

This repository contains my first implementation of a LLVM function pass able to check for boundary checks in arrays. Such passes are helpful to guide the developers during the development phases avoiding invalid accesses in buffers (e.g, underflows and overflows). The work is still in progress but this one in particular raises alerts at compilation time in case violations are found but it is also able to instrument the code to include runtime checks.

The implementation uses the GEP (GetElementPtr) instructions and extracts the index and array length to perform the analysis.

Example 01 (constant indexes and a static array) :
> %3 = getelementptr inbounds [1 x i32], __[1 x i32]* %2__, i64 0, __i64 1__

the pass retrieves the size of "[1 x i32]* %2" which means an array with length 1 (e.g., array[0]), and [i64 1] which means at position 1. In this case, a compilation error is triggered because array[1] is not allowed since the indexes are zero-based.

Limitations (dynamic array or dynamic indexes):
> getelementptr inbounds i32, __i32* %5__, i64 2

> getelementptr inbounds [10 x i32], [10 x i32]* @list, i64 0, __i64 %40__

The current implementation is still not able to handle dynamic indexes or dynamic pointers. This still needs to be overcome but is now under investigation! :-)

__Options available__
 - -force-runtime-checks: force runtime checks and ignore violations detected at compilation time;
 - -show-byte-code: it shows the byte code of the function after using the pass;
 - -optimize (to be implemented);
 - -debug
 
 __Using the Bounds Check Pass__
> make
> opt -load build/libLLVMBoundsCheck.so -bounds-check < <bytecode-file> <options>

 __Running the Regression Tests__
> make regression