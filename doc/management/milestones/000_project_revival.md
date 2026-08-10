# Milestone 000: Project revival

- **Status**: Ongoing

## Objective

Re-establish a trustworthy project baseline and the development systems needed to deliver the `0.1.0` roadmap consistently.

## Deliverables

- [x] Replace the dated goals, terminology, roadmap, and active milestone set.
- [x] Retain the GCC 16.1 development container and independent standard-reflection smoke check.
- [x] Establish Linux continuous integration for the reflection smoke check, project build, and tests.
  - Documented in README.md
- [x] Establish Doxygen coverage, line coverage, and branch coverage measurement.
  - **Discarded**: Scripts and other things is too much for MVP phase. Requirement stays, but no automated way to measure it yet.
- [x] Establish repeatable benchmark infrastructure without setting a performance target.
- [x] Set up a repository AI-development harness based on [Spec Kit](https://github.com/github/spec-kit).

## Boundaries

- Product code must be portable by design, but this milestone validates Linux with GCC 16.1 or newer.
- macOS development uses the Linux development container.
- The exact AI integration, configuration, templates, and workflows are decided while executing this milestone rather than by the product roadmap.

## Completion criteria

- A clean checkout can run the reflection smoke check, configure, build, and test through documented commands.
- Continuous integration enforces the supported Linux build and test path.
- Documentation coverage, test coverage, and benchmarks can be measured repeatably.
- The AI harness can take a feature from specification through planning, tasks, implementation, and validation using repository guidance.
