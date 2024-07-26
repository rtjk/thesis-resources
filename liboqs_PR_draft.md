
# Add CROSS

This pull request adds the CROSS signature scheme. CROSS is part of NIST's [Round 1 call for additional signatures][NIST_call]. It comes with a reference implementation and an AVX2-optimized one.

Starting from the latest NIST submission package ([version 1.2][NIST_submission]) we have:
- Added support for [parallel Keccak][CROSS_keccakx4] to speed up signing and verification.
- Applied a [few changes][CROSS_integration] to comply with liboqs' API and coding conventions.

Additionally, two changes are proposed to liboqs' test setup to incorporate CROSS:
- Upgraded Clang to version 18 (the latest stable) in  GithHub Actions. Valgrind's leak test (`test_leaks.py`) would fail on the parameter set `rsdp-192-avx2` when compiling on Ubuntu with clang-15. This seems specific to version 15, no leaks are detected with older and newer releases of Clang.
- Avoid using threads in `test_kem.py` and `test_sig.py` with algoriths that have `large-stack-usage: true`. This was previously achieved by manually listing such algorithms in an array.

`TODO: before PR edit test_kem, test_sig, workflows/unix.yml`

Thank you for considering this addition to the project. We hope it provides value and helps further the exploration of the on-ramp candidates.

* [ ] Does this PR change the input/output behaviour of a cryptographic algorithm (i.e., does it change known answer test values)?  (If so, a version bump will be required from *x.y.z* to *x.(y+1).0*.)
* [x] Does this PR change the list of algorithms available -- either adding, removing, or renaming? Does this PR otherwise change an API? (If so, PRs in fully supported downstream projects dependent on these, i.e., [oqs-provider](https://github.com/open-quantum-safe/oqs-provider) will also need to be ready for review and merge by the time this is merged.)

<!-- Once your pull request is ready for review and passing continuous integration tests, please convert from a draft PR to a normal PR, and request a review from one of the OQS core team members. -->

[NIST_call]: https://csrc.nist.gov/projects/pqc-dig-sig/round-1-additional-signatures
[NIST_submission]: https://www.cross-crypto.com/nist-submission.html
[CROSS_keccakx4]: https://github.com/rtjk/CROSS-keccak-times4
[CROSS_integration]: https://github.com/rtjk/CROSS-PQClean
[PR_CircleCI]: https://github.com/open-quantum-safe/liboqs/pull/1849


