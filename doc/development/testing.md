# Testing

- Always use test classes for runtime tests. Keep fixtures small and focused.
- Name cases `GIVEN_context_WHEN_action_THEN_expectation` or `WHEN_action_THEN_expectation`. Describe the observable expectation.
- Test one behavior per case with clear setup, action, and assertions.
- Prefer small, explicit, composable matchers over long sequences of assertions or helper functions that hide assertions.
- Reuse existing matchers before creating new ones. Keep reusable matchers in dedicated headers.
- Prefer whole-object equality when checking complete state. Provide readable value printers for useful failure diagnostics.
- Check prerequisites with fatal assertions before accessing dependent values.
- Identify parameterized inputs clearly so failures show which scenario failed.
- Verify observable behavior without depending on incidental implementation details.

## Coverage

Coverage is opt-in. In the GCC dev container, configure a separate build and run the `coverage` target:

```sh
cmake -S . -B build-coverage -G Ninja -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build-coverage --target coverage
```

The target builds tests, resets counters, runs CTest, and generates `build-coverage/coverage/index.html` with a line/branch summary in the
terminal. Reports include only sources under the project root, including tests. External system headers and third-party sources under
`3rd_party/` or any `_deps/` directory are excluded. No percentage gate is enforced. Reports are generated even when tests fail, but the
target still returns failure.
Collection errors also fail the target.

With `ENABLE_COVERAGE=ON`, all targets compile with GCC coverage instrumentation. Ordinary CTest runs accumulate raw counters; the coverage
target starts a fresh run. Compile-only checks have no runtime counters. Benchmarks contribute when executed and are not run automatically
by this target. Do not run tests concurrently with coverage collection in the same build tree.

The dev container supplies gcovr and gcov. Outside it, these tools must be available when enabling coverage; ordinary builds do not require
them. Rebuild an existing container to install newly added development tools.
