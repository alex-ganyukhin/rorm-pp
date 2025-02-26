# Milestone: Setup Clang with Reflection

- **Timeline**: Feb-Mar 2025
- **Status**: Ongoing

---

## Objectives
- **The primary objective** of this milestone is to prepare a compiler for further development.

---

## Deliverables
- [ ] The way to get and build clang with all the needed things is documented
- [ ] The minimal example of reflection is implemented, and buildable with the clang built according to the instructions

---

## Tasks
- [x] Build clang, libc++ with C++26 reflection enabled
- [x] Setup cmake project to use the built clang
- [ ] Create minimal working example of reflection
- [ ] Include google tests

---

## Problems
### Built clang compiler is requires a lot of additional "moves" to build & run
1. LD_LIBRARY_PATH is required to run the built -> have to use rpath instead

### Cannot use the installed clang, only the one from "build" directory
- ...

### Code analyzer warns about reflection things


---

## Risks
- N/A

---

## Decisions
- N/A

---

## Lessons learned
- N/A
