# Milestone 005: Release 0.1.0

- **Status**: Planned

## Objective

Integrate, document, package, and validate the completed feature set as an early-adopter release.

## Deliverables

- [ ] Provide an installable and consumable CMake package.
- [ ] Publish Doxygen documentation for the public API.
- [ ] Add focused guides only where API comments and tests do not provide an adequate learning path.
- [ ] Establish and publish repeatable baseline benchmarks without a competitive target.
- [ ] Document the supported environment, known limitations, and explicit exclusions.
- [ ] Prepare the Semantic Versioning `0.1.0` release.

## Completion criteria

- At least 95% of public APIs are covered by Doxygen comments.
- Line coverage and branch coverage are each at least 95% for the complete product.
- A subsystem scenario defines a Record, round-trips it through JSON, connects to an existing PostgreSQL schema, performs individual and batch CRUD, retrieves generated values, exercises every common typed-read category, uses an explicit transaction, and maps a parameterized raw-query result.
- The standalone reflection smoke check, clean project build, full test suite, package-consumer test, and benchmarks pass on the supported Linux/GCC toolchain.
