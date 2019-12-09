# BoundsCheck LLVM Pass

This repository contains my first implementation of a LLVM function pass able to check for boundary checks in arrays. Such passes are helpful to guide the developers during the development phases avoiding invalid accesses in buffers (e.g, underflows and overflows). The work is still in progress but this one in particular raises alerts at compilation time in case violations are found but it is also able to instrument the code to include runtime checks.

The implementation uses the GEP (GetElementPtr) instructions and extracts the index and array length to perform the analysis.

#### Example 01 (constant indexes and static arrays)
> getelementptr inbounds [1 x i32], __[1 x i32]* %2__, i64 0, __i64 1__

the pass retrieves the size of *[1 x i32] %2* which means an array with length 1 (e.g., array[0]), and [i64 1] which means at position 1. In this case, a compilation error is triggered because array[1] is not allowed since the indexes are zero-based.

#### Example 02 (static arrays and dynamic index offset)

> getelementptr inbounds [1 x i32], __[1 x i32]* %2__, i64 0, __i64 %43__

like in the previous example, the pass retrieves the array's size which is 1. However, the *index* is not a constant but an offset which cannot be checked at compilation time. The pass transforms the byte code and adds a check at runtime which if violated aborts the execution and shows more information to the user.

#### Limitation (dynamic allocated arrays):

> getelementptr inbounds i32, __i32* %5__, i64 2

The current implementation is still not able to handle dynamic pointers. This check especially is a bit tricky since looking at the LLVM byte code the only place we have information about the size is at the __malloc__ instruction. One strategy that could be used is trying backing tracking until the allocation instruction and retreving the declared size. However, in most of the real scenarios it won't be suitable. Pointers are usually passed as function arguments and implementing this might be too costly or not possible.

#### Options available
 - -force-runtime-checks: force runtime checks and ignore violations detected at compilation time;
 - -show-byte-code: it shows the byte code of the function after using the pass;
 - -optimize (to be implemented);
 - -debug
 
#### Using the Bounds Check Pass
> make
> opt -load build/libLLVMBoundsCheck.so -bounds-check < bytecode-file > < options >

 #### Running the Regression Tests
> make regression