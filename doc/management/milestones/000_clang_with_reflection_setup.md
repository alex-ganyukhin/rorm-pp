# Milestone: Setup Clang with Reflection

- **Timeline**: Feb-Mar 2025
- **Status**: Done

---

## Objectives
- **The primary objective** of this milestone is to prepare a compiler for further development.

---

## Deliverables
- [x] The way to get and build clang with all the needed things is documented
- [x] The minimal example of reflection is implemented, and buildable with the clang built according to the instructions

---

## Tasks
- [x] Build clang, libc++ with C++26 reflection enabled
- [x] Setup cmake project to use the built clang
- [x] Create minimal working example of reflection
- [x] Include google tests
- [x] Document
- [x] Grand cleanup of library and cmake files

---

## Problems
### [SOLVED] LD_LIBRARY_PATH is required to run the built binaries
 - have to use rpath instead
 - Done, via `CMAKE_BUILD_RPATH`

### [HALF-SOLVED] Cannot use the installed clang, only the one from "build" directory
- User can use `-DCOMPILER_DIR=/abs/path/to/bloomberg/clang/build` to specify the path to the clang build directory

### [DONE] Code analyzer warns about reflection things
- Likely caused by the fact that this is a reflection draft, not supported by linters etc...
- So far, left as is

---

## Risks
- N/A

---

## Decisions
- N/A

---

## Lessons learned
- N/A
