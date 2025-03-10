# Project roadmap

## 2025-Q1-Q2: Setup and Model

**Timeline**: Feb-Jul 2025

### Goals

- Setup the project
- Design the architecture of the solution
- Given a class/struct defined by user, the library should be able to analyze it
  - What member fields are?
  - What attributes of the members are? (e.g. `[[orm::column("name")]]`)
  - Can custom attributes be used at all, are they visible?
- Given a class/struct defined by user, the library should be able to serialize/deserialized it to/from JSON.


### Milestones

- [x] [Set up the compiler](./milestones/000_clang_with_reflection_setup.md)
- [ ] [Model to handle JSON](./milestones/001_the_first_steps.md)


---

## 2025-Q3-Q4: Basic ORM

**Timeline**: Aug-Dec 2025

### Goals
- Implement basic ORM capabilities
  - Basic and simple CRUD operations
  - *Relationships* are planned for future releases

### Milestones
- [ ] [Simple ORM](./milestones/002_simple_orm.md)

---

## Ideas for future releases
- Relationships support
- Migrations
