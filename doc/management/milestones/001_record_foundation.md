# Milestone 001: Record foundation

- **Status**: Planned

## Objective

Provide the reflection-based Record model shared by JSON interchange and database persistence.

## Deliverables

- [ ] Discover persisted properties exposed as public fields.
- [ ] Discover explicitly marked, conventional getter and setter properties.
- [ ] Allow getter-only and setter-only properties in operations that require only the available direction.
- [ ] Support mapping overrides for names, primary and generated values, exclusions, and custom value conversions.
- [ ] Support booleans, fixed-width integers, floating-point values, strings, bytes, enums, optional/null, and common date/time values.
- [ ] Reject invalid Record definitions, unsupported values, and incompatible operations during compilation where possible.
- [ ] Review the Record concepts against anticipated relationship support without implementing relationships.

## Completion criteria

- Public-field and accessor-backed Records expose the same mapping capabilities where their supported directions overlap.
- Invalid C++ mappings covered by the milestone fail during compilation with actionable diagnostics.
- Public APIs satisfy the [project quality objectives](../goal.md#quality-objectives).
