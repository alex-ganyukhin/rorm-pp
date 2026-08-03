# Milestone 002: JSON interchange

- **Status**: Planned

## Objective

Serialize and deserialize Records for interchange without turning rorm-pp into a general-purpose JSON library.

## Deliverables

- [ ] Serialize Records using readable properties.
- [ ] Deserialize Records using writable properties.
- [ ] Support nested reflected values, optional values, and common sequences recursively.
- [ ] Apply names, exclusions, accessor directions, and custom value conversions consistently.
- [ ] Report malformed input, missing required data, and incompatible values clearly.

## Boundaries

- JSON support applies to Records and their nested reflected values, not arbitrary unrelated C++ types.
- JSON interchange does not imply first-class PostgreSQL JSON or JSONB column persistence.

## Completion criteria

- Representative Records round-trip through JSON without data loss.
- Direction-specific and failing conversions have unit and subsystem coverage.
- Public APIs satisfy the [project quality objectives](../goal.md#quality-objectives).
