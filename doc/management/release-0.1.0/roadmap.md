# Release 0.1.0 roadmap

This roadmap grows the library from a working development foundation to a packaged ORM. The authoritative release scope lives in [Release 0.1.0](./release.md).

The [2025 plan](../archive/2025/roadmap.md) is retained as project history.

## Implementation order

`000 → 001 → 002 → 003 → 004 → 005`

| Milestone | Expected outcome |
| --- | --- |
| [000: Development foundation](./000-foundation-milestone.md) | A clean checkout has one documented toolchain, validation path, benchmark harness, and Spec Kit workflow. |
| [001: Public API planning](./001-api-milestone.md) | The intended `0.1.0` user experience is documented before implementation constrains it. |
| [002: Record and JSON](./002-json-milestone.md) | A supported Record round-trips through JSON using reflection-based mapping. |
| [003: PostgreSQL backend](./003-postgresql-backend-milestone.md) | PostgreSQL executes internal structured backend operations with typed values, results, errors, and transactions. |
| [004: Typed ORM](./004-typed-orm-milestone.md) | Users perform typed single-table CRUD and common reads without constructing backend operations. |
| [005: Release readiness](./005-release-milestone.md) | Another CMake project can consume a documented, tested, packaged `0.1.0` release. |

Every milestone is subject to the [project quality objectives](../goal.md#quality-objectives). Milestones 001 and 002 are detailed because they are the current planning horizon; later milestone details are deferred until that work becomes current.
