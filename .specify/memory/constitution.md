<!--
Sync Impact Report
- Version change: 1.0.0 -> 2.0.0
- Modified principles: none
- Added sections: none
- Removed rules:
  - per-work-cycle test cadence
  - before-handoff validation timing
- Updated artifacts:
  - ✅ AGENTS.md
  - ✅ .specify/templates/tasks-template.md
- Reviewed without changes:
  - ✅ .specify/templates/plan-template.md
  - ✅ .specify/templates/spec-template.md
  - ✅ other installed Spec Kit skills
- Follow-up TODOs: none
-->
# rorm-pp Constitution

## Core Principles

### I. Reflection-Native Type Safety

Mappings MUST derive from standard C++ reflection rather than macros, generated
registration code, or parallel schemas. Invalid mappings and typed operations MUST fail
at compile time whenever the necessary information is available. Failures that depend on
runtime data or database state MUST produce clear, actionable diagnostics. Reflection is
the project's defining capability, and type safety is its primary benefit.

### II. Tests and API Documentation Are Delivery

Every behavior change MUST include tests for its successful behavior and relevant
failures. Tests MUST be readable, representative uses of the public behavior: they are
both evidence and executable documentation. Public APIs MUST have Doxygen documentation.
A change is incomplete when its required tests or API documentation are missing.

### III. Code Quality Is Non-Negotiable

Code MUST remain focused, readable, maintainable, consistently formatted, and clean under
the required warning policy. Diagnostics MUST be fixed rather than suppressed or weakened.
New complexity, dependencies, indirection, or customization points MUST address a current,
demonstrated requirement. Changes MUST avoid unrelated edits. When code quality conflicts
with an optimization during PoC or MVP development, code quality MUST win.

### IV. Consistent Developer Experience

Public APIs, terminology, defaults, diagnostics, and equivalent operations MUST follow
established project conventions. New behavior MUST reuse those conventions or document a
specific reason for deviating. Equivalent success and failure paths MUST behave
consistently and MUST be covered by tests. Consistency reduces the amount users must learn
and makes the library predictable.

### V. Performance Follows Product Maturity

The project MUST maintain repeatable benchmark infrastructure during PoC and MVP work, but
performance targets MUST NOT gate delivery at those stages unless a measured bottleneck
blocks intended use. Straightforward optimizations that preserve clarity and
maintainability MAY be included. Performance claims and complexity justified by
optimization MUST be backed by repeatable measurements. Strong performance targets belong
to later product maturity.

## Quality Gates

Each delivered change MUST satisfy all applicable gates:

- at least 95% of public APIs covered by Doxygen documentation;
- at least 95% line coverage and 95% branch coverage;
- unit and subsystem tests for successful behavior and relevant failures;
- clean `-Wall -Wextra -Werror` builds under the required compiler; and
- repeatable benchmark evidence for performance claims or performance-driven complexity.

When a gate is not applicable, cannot be measured, or cannot run in the available
environment, the project-health report MUST identify the affected gate and the reason. An
unrun gate MUST NOT be reported as passing.

## Development Workflow

Plans MUST pass a Constitution Check before implementation and again after design. Tasks for
behavior changes MUST include tests; tasks that add or change public APIs MUST include
Doxygen documentation. Project health MUST be assessed through the standalone reflection
smoke check, a warning-clean project build, the complete test suite, the applicable coverage
and documentation gates, and benchmark evidence for performance claims. `AGENTS.md` defines
the current commands and repository-specific health checklist.

Reviewers MUST verify the five principles and applicable quality gates. Any deliberate
exception MUST be documented with its scope, evidence, and removal condition before the
change is accepted.

## Governance

This constitution is the highest authority for project governance. `AGENTS.md`, Spec Kit
templates, and other execution guidance MUST remain consistent with it. Management
documents MAY reference this constitution, but this constitution and its execution artifacts
MUST NOT depend on management documents.

Amendments require an explicit rationale, an impact report, and updates to every affected
execution artifact. Constitution versions follow Semantic Versioning: MAJOR for incompatible
principle or governance changes, MINOR for new or materially expanded rules, and PATCH for
clarifications. Every plan and review MUST verify compliance with the current version.

**Version**: 2.0.0 | **Ratified**: 2026-08-11 | **Last Amended**: 2026-08-11
