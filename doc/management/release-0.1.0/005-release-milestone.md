# Milestone 005: Release readiness

## Objective

Make the completed feature set usable by another CMake project as release `0.1.0`.

## Expected outcome

An early adopter can install rorm, discover its supported API, run a complete PostgreSQL example, and understand the release's limitations without reading repository internals.

## Completion criteria

- [ ] `cmake --install` produces a package that a separate minimal CMake project can consume without repository-relative paths.
- [ ] Published Doxygen output documents at least 95% of public APIs; focused guides exist only for workflows that API comments and tests do not explain adequately.
- [ ] Line coverage and branch coverage are each at least 95% for the complete product.
- [ ] One subsystem scenario defines a Record, round-trips it through JSON, connects to an existing PostgreSQL schema, performs individual and batch CRUD, retrieves generated values, exercises every common typed read, and uses an explicit transaction.
- [ ] The dummy benchmark is replaced by repeatable baseline benchmarks for the implemented public operations; no competitive target is required.
- [ ] The README states the supported compiler/platform, installation and validation commands, known limitations, and explicit exclusions.
- [ ] From a clean supported environment, the reflection smoke check, warning-clean build, full test suite, package-consumer test, and benchmarks all pass.
- [ ] The release is tagged `0.1.0` according to Semantic Versioning.

## Out of scope

- New product capabilities added only to make the release appear larger.
- Package-registry publication, performance claims, or stability guarantees beyond Semantic Versioning for `0.x` releases.
