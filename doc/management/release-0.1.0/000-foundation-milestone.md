# Milestone 000: Development foundation

## Objective

Make a clean checkout ready for repeatable human- and AI-assisted development.

## Expected outcome

The repository explains what is being built, how to build and validate it in the supported environment, and how an AI agent takes a feature from specification to implementation. No product feature is delivered by this checklist.

## Completion criteria

- [x] The obsolete 2025 plan is archived and the active goal, release scope, roadmap, terminology, and milestones agree with one another.
- [x] `bash .devcontainer/post-create.sh` independently proves that GCC 16.1 can compile C++26 reflection code.
- [x] `README.md` gives the commands to configure, build, and run the test suite from a clean development container.
- [x] An optional benchmark target builds and runs through the commands documented in `README.md`; the dummy benchmark is only a harness check.
- [x] Spec Kit is initialized with the project constitution, Codex skills, templates, and the specification-to-implementation workflow.
- [ ] The complete smoke-check, build, and test sequence has been rerun successfully from a clean development container after the revival changes.

## Out of scope

- Hosted continuous integration. Add it when a hosted runner with the required reflection compiler is useful and available.
- Automated Doxygen, line-coverage, and branch-coverage measurement. The quality requirements remain authoritative, but automation is deferred during MVP work.
- Meaningful performance measurements. Milestone 005 replaces the dummy benchmark with release baselines.
- Native macOS or Windows validation; development on macOS uses the Linux development container.
