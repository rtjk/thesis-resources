
State of parallel Keccak implementation

##

**Problem description:**

The files:

`liboqs/src/common/pqclean_shims/fips202x4.c`
`liboqs/src/common/pqclean_shims/fips202x4.h`

are supposed to be shims for a parallel version of Keccak in PQClean. However, it appears that PQClean currently lacks a Keccak-x4 implementation, as evidenced by the absence of symbols like `shake128x4_inc_absorb` in the PQClean codebase.

Furthermore, in liboqs these symbols point (through a series of defines and callbacks) to the function definitions in:

`liboqs/src/common/sha3/xkcp_sha3x4.c`

However it appears that this implemetation does not take full advantage of the AVX2 parallelism offered by:

`XKCP/lib/low/KeccakP-1600-times4/AVX2`

For example during absorbtion bytes are always added with `AddBytes` insted of using `AddLanesAll` where possible.

**Proposal:**

- Files `fips202x4.c` and `fips202x4.h` could be moved from `pqclean_shims` to  `common/sha3` since they are not actually available in PQClean
- The absorbtion and squeezing phases in `xkcp_sha3x4.c` can be modified slightly to take advantage of `AddLanesAll` and `ExtractLanesAll` similar to [this implementation](https://github.com/rtjk/keccak-parallel-incremental/blob/main/my_par_keccak.c).

I can prepare a PR if these seem like helpful changes, looking forward to your feedback!





