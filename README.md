### BoundsChecks LLVM Pass

This small project contains a simple implementation of a LLVM function pass able to check for boundary checks in arrays. Such pass is helpful to avoid invalid accesses (e.g, underflows and overflows of buffers) and alert the user at compilation time about these kind of violations.

Currently, this repo is still empty but the idea is to cover:
- simple assignments of arrays;
- assignments in static arrays;
- assignments in dynamic allocated arrays;
- warnings at compilation time;
- asserts at run time.

That is for now. More to come later. :-)
