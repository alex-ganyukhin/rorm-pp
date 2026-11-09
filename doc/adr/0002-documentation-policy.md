---
Status: Active
Date: 2026-11-09
---
# Keep API contracts in code and Doxygen

Before implementation, Markdown specifications and milestone plans define the intended API and behavior. Once implemented, code and Doxygen
are the primary source of truth; Markdown usage guides are secondary explanations of workflows. This keeps contracts beside the code and
avoids competing API references.

## Required policy

- Transfer implemented contracts into Doxygen and retire duplicated reference material, preserving decision rationale and
  still-unimplemented scope. Declarations and dummy scaffolds do not establish implemented behavior.
- Surface and resolve disagreements between plans, code, and documentation. Current implementation behavior alone does not invalidate a
  requirement.
- Public APIs require concise Doxygen documentation and local usage examples. Detail must be proportional to contract complexity, not an
  arbitrary length. Explain complex contracts fully, and consider whether the API can be simplified.
- Assume general C++ knowledge, but explain rorm conventions and unfamiliar annotation or reflection syntax.
- A type's example may cover its enum values and simple fields. Callable members and independent annotations need their own examples. Very
  obvious getters may omit the example; setters have no blanket exemption. Overloads may reference the primary version's example with
  `@see`.
- Shared examples explain how APIs interact; they must not replace local examples with a catalogue that requires repeated navigation.
- Document non-obvious internal contracts with Doxygen, without requiring usage examples. Implementation comments explain reasons,
  invariants, or surprising choices rather than narrating straightforward code.

Follow the [documentation writing guide](../development/documentation.md) when writing or changing documentation. These requirements apply
to new or changed APIs; bulk migration of existing documentation is separate work.

## Alternatives and consequences

Keeping the full API reference in Markdown separates contracts from their code and invites drift. Requiring exhaustive comment sections for
every API creates boilerplate; requiring only a brief leaves unfamiliar usage unexplained. We instead require examples and add contract
detail when informative.

A contributor guide serves humans and agents. AGENTS.md makes following it mandatory without repeating it. A separate skill adds no
necessary capability for these standing rules. Documentation generation tooling is a separate concern.
