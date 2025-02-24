# Project roadmap

## 2025-Q1-Q2: Setup and PoC

**Timeline**: Jan-Jul 2025

### Goals

- Setup the project
- Given a class/struct defined by user, the library should be able to analyze it
  - What member fields are?
  - What attributes of the members are? (e.g. `[[orm::column("name")]]`)
  - Can custom attributes be used at all, are they visible?
- Given a class/struct defined by user, the library should be able to serialize/deserialized it to/from JSON.


### Milestones & Deliverables

- [ ] The project is set up
- [ ] The library can analyze a class/struct
- [ ] The library can serialize/deserialize to/from JSON

---

## 2025-Q3-Q4: Basic ORM

**Timeline**: Aug-Dec 2025

### Goals
- Design the architecture of the solution
- Implement basic ORM capabilities
  - CRUD operations
  - Basic queries
  - Relationships are planned for future releases

### Milestones
- [ ] The architecture is designed
- [ ] CRUD operations are implemented


---

## Ideas for future releases
- Relationships support
- Migrations
