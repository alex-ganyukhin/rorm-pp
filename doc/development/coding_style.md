# Coding style

- Please see `.clang-format` for the coding style.
- TODO things about class naming, functions naming, etc...

## Public function return types

Non-template public functions, including member functions, must state their return type explicitly. Do not use body-deduced `auto` or
`decltype(auto)` return types. Both leading return types (`Result foo()`) and explicit trailing return types (`auto foo() -> Result`) are
allowed. Constructors, destructors, and conversion functions retain their normal syntax.

This rule does not restrict return-type deduction in templates or the use of `auto` for local variables.

## Licensing
- All the code in this repository is licensed under the Apache License, Version 2.0
- Please include the snippet below into all the files in this repository:

**C++**
```cpp
/**
 * @file    [Name of the file]
 *
 * Copyright [year] Aleksandr Ganiukhin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
```

**Other Languages**
```text
[Name of the file]

Copyright [year] Aleksandr Ganiukhin

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
