# Writing documentation

Follow [ADR 0002](../adr/0002-documentation-policy.md). This guide applies to API comments, implementation comments, and Markdown
documentation.

## Start with the reader and the source of truth

- Assume general C++ knowledge. Explain rorm conventions and unfamiliar C++26 annotation/reflection syntax rather than teaching ordinary C++
  constructs.
- Write for the reader's task. Be terse and concrete, but include everything needed to use an API safely. Do not restate signatures or
  straightforward code.
- Before implementation, write intended contracts in Markdown plans and specifications. Clearly label provisional decisions and incomplete
  behavior.
- Once behavior is implemented, code and Doxygen are the primary reference. Move its contract into Doxygen and retire duplicated reference
  material; preserve rationale and unimplemented scope. A declaration or dummy scaffold does not make behavior implemented.
- Use secondary Markdown guides for workflows and explanations. Link to the primary API documentation instead of maintaining another
  reference. Workflow examples may call APIs without copying their full contracts.
- Resolve discrepancies explicitly: implementation behavior is not by itself proof that a planned requirement was wrong. Update affected
  documentation together with API or behavior changes; make no unsupported availability claims.

## Line widths

Wrap documentation prose, including Markdown and Doxygen comments, at 140 columns, counting indentation and comment prefixes. Format C++
code at 120 columns, including code examples. Do not wrap documentation at 80 columns.

The repository uses `ColumnLimit: 120` and `ReflowComments: false` in `.clang-format`. Comment wrapping and alignment are the author's
responsibility; disabling comment reflow preserves the separate documentation width when formatting code.

## Document public APIs

Public means supported, user-facing APIs, including types, functions, constants, annotations, and callable members. C++ accessibility alone
does not determine whether an entity is public API.

Place Doxygen beside the declaration. Start with `@brief` and provide a local usage example, even for simple APIs. Very obvious getters may
omit the example; setters have no blanket exemption. Do not leave empty tags or filler prose.

Add detail in proportion to contract complexity. Explain relevant constraints, compile-time requirements, side effects, ownership,
lifetimes, result semantics, and failures. Fully document a complex contract, then consider simplifying the API rather than hiding its
complexity with a shorter comment.

Use this order, omitting sections that add no useful information:

1. `@brief`: a quick description of the entity's purpose.
2. Details: behavior or constraints not already clear from the brief and example.
3. `@code{.cpp}` … `@endcode`: a small, realistic usage example.
4. `@tparam` and `@param`: semantic requirements and parameter meanings.
5. `@throws`: actual exception conditions, only when applicable.
6. `@return`: useful result semantics, including returned error conditions.

Returned errors and thrown exceptions are different contracts. Do not invent exceptions or guarantees. A void result needs no `@return`.
Simple APIs may need only a brief and example; parameter entries are not mandatory boilerplate. See the [Doxygen command
reference](https://www.doxygen.nl/manual/commands.html).

### Format comments for readability

Use multiline `/** … */` blocks with vertically aligned leading `*` characters. Use spaces for alignment and one blank comment line between
logical sections. Keep ordinary prose paragraphs left-aligned after the `* ` prefix.

Align prose-tag payloads to the same column within a comment, leaving at least two spaces after the longest prose tag. Code delimiters such
as `@code{.cpp}` and `@endcode` stand on their own lines and do not determine this column. Within each parameter, template-parameter, or
exception group, align names and descriptions independently of the other groups, with at least two spaces between columns. Align wrapped
descriptions beneath their first line. A long exception name must not push parameter descriptions to the right.

Indent code examples by four spaces after the `* ` prefix, then follow the repository's C++ indentation rules within the example. Keep
sections optional; spacing must not introduce empty tags or unnecessary text.

Continue using `clang-format` with the existing repository configuration. Authors are responsible for correct comment spacing, alignment,
and readability, including checking comments after running the formatter. Reviewers must check compliance with this guide; a formatter pass
alone does not establish compliance. No additional formatter or custom formatting script is required.

### Minimal public comment

This illustrates documentation for the intended annotation contract, without claiming that the current scaffold implements it:

```cpp
/**
 * @brief  Override a field or enumerator's logical name.
 *
 * @code{.cpp}
 *     struct Profile
 *     {
 *         [[= rorm::name("displayName")]] std::string display_name;
 *     };
 * @endcode
 */
consteval auto name( std::string_view value );
```

### Expanded public comment

This is a writing template, not an additional rorm API. Replace the placeholders with the actual contract and remove sections that are
unnecessary:

```cpp
/**
 * @brief   <Purpose of the operation.>
 *
 * <Relevant constraints, effects, ownership, and lifetime requirements.>
 *
 * @code{.cpp}
 *     // A small example showing the actual call and relevant result handling.
 * @endcode
 *
 * @tparam  T       <Semantic requirements on T.>
 *
 * @param   input   <Meaning and applicable constraints.>
 * @param   policy  <Meaning of the policy.>
 *                  <Continuation aligned with its description.>
 *
 * @throws  ExceptionType  <Actual exception type and the condition that throws it.>
 *
 * @return  <Success value and returned failure conditions.>
 */
```

### Choose the example's boundary

- Give each public type its own example. Its example may cover enum values and simple fields; describe their semantics locally without
  repeating snippets.
- Give callable members and independently usable annotations their own examples. Show the annotation syntax and what it accomplishes.
- For overloads, put an example on the primary version and use `@see` on the others. Describe overload-specific differences locally.
- Use shared examples to explain interactions between APIs. Do not replace local examples with a library of presets that readers must
  repeatedly navigate.
- Keep examples consistent with actual signatures and behavior. Clearly identify planned or illustrative examples, and do not claim they
  compile or run unless verified with the supported toolchain.

## Document internal APIs and implementation choices

Private members and implementation APIs such as `rorm::detail` are internal, even when their members use C++ `public` access.

Use Doxygen for non-obvious contracts: requirements a maintainer needs to call or modify a helper safely, hidden invariants, side effects,
and lifetime assumptions. Straightforward internal types and functions need no ceremonial comments. Internal usage examples are not
required; aim for APIs that do not need them.

For example, a hypothetical internal helper may need only its hidden precondition:

```cpp
/**
 * @brief  Find a property in a mapping whose names have already been validated as unique.
 */
```

Inside implementations, explain why a choice exists or which invariant must hold. Do not narrate obvious operations. Keep the explanation
beside the code it concerns; use an ADR for durable decision rationale.

## Review before finishing

- Is planned versus implemented behavior clear, and is the contract in the right place?
- Does each public API have a useful brief and the required local example?
- Are unfamiliar annotation/reflection conventions explained without teaching basic C++?
- Are necessary constraints, failures, effects, and lifetime rules covered without filler?
- Are overload references useful, and shared examples limited to interactions?
- Are internal contracts and surprising choices explained without redundant comments?
- Do examples match the API, links resolve, and changed behavior have matching documentation?
- After running `clang-format`, are comment columns, continuation lines, and section spacing correct?

Apply these rules to new or changed APIs. Migrating all existing documentation and adding generation tooling are separate tasks.
