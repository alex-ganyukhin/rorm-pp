# Milestone 001: Public API planning

## Objective

Define the intended `0.1.0` public API as reusable user documentation before implementation choices make it expensive to change.

## Expected outcome

Substantive `api.md` documentation shows how a user defines Records, exchanges JSON, connects and transacts, and performs ORM operations. Record and JSON usage is precise enough to implement milestone 002; later database and ORM usage may remain provisional.

The milestone may use documentation, prototypes, or both to reach that outcome. It does not require unused public scaffolding, and an empty placeholder `api.md` does not count.

## Detailed plan

- [x] Write representative user journeys for Record definition, JSON round-tripping, connection and transaction control, typed CRUD, and typed reads.
- [x] Specify Record fields and accessor properties, including mapped names, exclusions, keys, generated values, directionality, and custom conversions.
- [x] Specify supported value categories and how invalid mappings are reported.
- [x] Specify JSON serialization, deserialization, errors, and ownership/return-value conventions precisely enough for milestone 002.
- [x] Sketch PostgreSQL and ORM usage across the complete release scope, marking undecided details as provisional rather than presenting them as a stable contract.
- [x] Make terminology, naming, equivalent operations, and failure handling consistent across the examples.
- [x] Review `api.md` against [release.md](./release.md), the project goals, and the project glossary.

## Completion criteria

- [x] `api.md` is useful as user documentation rather than as an internal design log.
- [x] Every `0.1.0` feature appears in at least one representative usage flow.
- [x] Record and JSON sections leave no public-interface decisions for milestone 002.
- [x] PostgreSQL and ORM sections communicate direction without claiming unresolved details are final.
- [x] No code artifact exists solely to make the milestone appear implemented.

### Completion Notes

- Let API be C++-like definition of the namespace, and that's it. Later we will drop api.md, as soon as the items mentioned there will be implemented in the code


## Out of scope

- Implementing Record, JSON, PostgreSQL, or ORM behavior.
- Finalizing detailed APIs for unscheduled future features.
