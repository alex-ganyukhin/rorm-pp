#Repository guide

## Project

`rorm-pp` is a header-only C++26 ORM prototype built around standard reflection. Public code belongs in `rorm/include/rorm/`, tests in `tests/`, CMake helpers in `cmake/` or `3rd_party/`, and project notes in `doc/`.

Use the GCC dev container for development. The project requires GCC 16.1 or newer and must be compiled with `-std=c++26 -freflection`.

## Build and test

From the repository root, validate the container's reflection compiler independently of rorm with:

```sh
bash .devcontainer/post-create.sh
```

Configure, build, and test the project separately with:

```sh
cmake -S . -B build-devcontainer -G Ninja -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build-devcontainer --parallel
ctest --test-dir build-devcontainer --output-on-failure
```

CMake fetches GoogleTest during configuration. Do not edit or commit generated build trees or fetched dependency sources.

## Change guidelines

- Keep public APIs in the `rorm` namespace and preserve the interface-library design unless the change explicitly requires compiled sources.
- Add or update GoogleTest coverage for behavior changes. Register new test source files in `tests/CMakeLists.txt`.
- Follow `.clang-format`; format changed C++ files with the standalone `clang-format` installed in the dev container.
- Keep `-Wall -Wextra -Werror` clean. Fix diagnostics rather than weakening the warning policy.
- Add the repository's Apache-2.0 copyright header to new source, CMake, and script files; see `doc/development/coding_style.md`.
- Keep changes focused and avoid unrelated edits, especially in generated `build*` directories.

## Healthy project checklist

- [ ] Changed files are formatted via `clang-format`
- [ ] Code coverage is above 95% for lines and branches
- [ ] All public APIs are documented with Doxygen, except obvious things like plain getters and setters
- [ ] No failed tests
- [ ] No build warnings or errors
- [ ] `doc/quickstart/*` are up to date with API changes
