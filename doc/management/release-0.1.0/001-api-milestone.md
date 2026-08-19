# Milestone 001: Public API planning

## Objective

Define the intended `0.1.0` public API as reusable user documentation before implementation choices make it expensive to change.

## Expected outcome

Substantive `api.md` documentation shows how a user defines Records, exchanges JSON, connects and transacts, and performs ORM operations. Record and JSON usage is precise enough to implement milestone 002; later database and ORM usage may remain provisional.

The milestone may use documentation, prototypes, or both to reach that outcome. It does not require unused public scaffolding, and an empty placeholder `api.md` does not count.

## Detailed plan

- [ ] Write representative user journeys for Record definition, JSON round-tripping, connection and transaction control, typed CRUD, and typed reads.
- [ ] Specify Record fields and accessor properties, including mapped names, exclusions, keys, generated values, directionality, and custom conversions.
- [ ] Specify supported value categories and how invalid mappings are reported.
- [ ] Specify JSON serialization, deserialization, errors, and ownership/return-value conventions precisely enough for milestone 002.
- [ ] Sketch PostgreSQL and ORM usage across the complete release scope, marking undecided details as provisional rather than presenting them as a stable contract.
- [ ] Make terminology, naming, equivalent operations, and failure handling consistent across the examples.
- [ ] Review `api.md` against [release.md](./release.md), the project goals, and the project glossary.

## Completion criteria

- [ ] `api.md` is useful as user documentation rather than as an internal design log.
- [ ] Every `0.1.0` feature appears in at least one representative usage flow.
- [ ] Record and JSON sections leave no public-interface decisions for milestone 002.
- [ ] PostgreSQL and ORM sections communicate direction without claiming unresolved details are final.
- [ ] No code artifact exists solely to make the milestone appear implemented.

## Out of scope

- Implementing Record, JSON, PostgreSQL, or ORM behavior.
- Finalizing detailed APIs for unscheduled future features.
