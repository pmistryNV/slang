
---
layout: user-guide
---

Modules and Access Control
===========================

While the preprocessor `#include`s is still supported, Slang provides a _module_ system for software engineering benefits such as clean expression of sub-component boundaries and dependencies, hiding implementation details, and providing a path towards true separate compilation.


### Defining a Module

A module in Slang comprises one or more files. A module must have one and only one primary file that is used as the source-of-truth to uniquely identify the module. The primary file must start with `module` declaration. For example, the following code defines a module named `scene`:

```
// scene.slang

module scene;

// ...
```

A module can contain more than one file. The additional files are pulled into the module with the `__include` syntax:

```
// scene.slang

module scene;

__include "scene-helpers";

```
```
// scene-helpers.slang

implementing scene;
// ...
```

The files being included into a module must start with `implementing <module-name>` declaration.

Note that the `__include` syntax here has a different meaning than the preprocessor `#include`. `__include` has the following semantics:
1. The preprocessor state at which a file inclusion does not apply to the file being included, and the preprocessor state after parsing the included file will not be visible to the outer "includer" file. For example, `#define`s before a `__include` is not visible to the included file, and `#define`s in the included file is not visible to the file that includes it.
2. A file will be included into the current module exactly once, no matter how many times a `__include` of that file is encountered.
3. Circular `__include`s are allowed, given (2).
4. All files that become part of a module via `__include` can access all other entities defined in the same module, regardless the order of `__include`s.

This means that the following code is valid:

```
// a.slang
implementing m;
void f_a() {}

// b.slang
implementing "m"; // alternate syntax.
__include a; // pulls in `a` to module `m`.
void f_b() { f_a(); }

// c.slang
implementing "m.slang"; // alternate syntax.
// OK to use f_a and f_b because they are part of module `m`, even
// if we are not including `a` and `b` here.
void f_c() { f_a(); f_b(); }

// m.slang
module m;
__include m; // OK, a file including itself is allowed and has no effect.
__include "b"; // Pulls in file b (alternate syntax), and transitively pulls in file a.
__include "c.slang"; // Pulls in file c, specifying the full file name.
void test() { f_a(); f_b(); f_c(); }
```

Note that both `module`, `implementing` and `__include` support two flavors of syntax to refer to a module or a file: either via 
normal identifier tokens or via string literals. For example, the following flavors are equivalent and will resolve to the same file:
```
__include dir.file_name; // `file_name` is translated to "file-name".
__include "dir/file-name.slang";
__include "dir/file-name";
```

> #### Note ####
> When using the identifier token syntax, Slang will translate any underscores(`_`) to hyphenators("-") to obtain the file name.

### Importing a Module

At the global scope of a Slang file, you can use the `import` keyword to import another module by name:

```hlsl
// MyShader.slang

import YourLibrary;
```

This `import` declaration will cause the compiler to look for a module named `YourLibrary` and make its declarations visible in the current scope. Similar to `__include`, `import` also supports both the identifier-token and the file-name string syntax.

You can only `import` a primary source file of a module. For example, given:
```
// m.slang
module m;
__include helper;

// helper.slang
implementing m;
// ...
```
It is only valid for the user code to `import m`. Attempting to `import helper` will result a compile-time error. 

Multiple `import`s of the same module from different input files will only cause the module to be loaded once (there is no need for "include guards" or `#pragma once`).
Note that preprocessor definitions in the current file will not affect the compilation of `import`ed code, and the preprocessor definitions in the imported code is not visible to the current file.

> #### Note ####
> Future versions of the Slang system will support loading of modules from pre-compiled binaries instead of source code.
> The same `import` keyword will continue to work in that case.

### Access Control

Slang supports access control modifiers: `public`, `internal` and `private`. The module boundary plays an important role in access control.

`public` symbols are accessible everywhere: from within the different types, different files or different modules.

`private` symbols are only visible to other symbols in the same type. The following example shows the scope of `private` visibility:
```csharp
struct MyType
{
    private int member;

    int f() { member = 5; } // OK.

    struct ChildType
    {
        int g(MyType t)
        {
            return t.member; // OK.
        }
    }
}

void outerFunc(MyType t)
{
    t.member = 2; // Error, `member` is not visible here.
}
```

`internal` symbols are visible throughout the same module, regardless if it is referenced from the same type or same file. But they are not visible to other modules. The following example shows the scope of `internal` visibility:

```csharp
// a.slang
module a;
__include b;
internal void f() { f_b(); } // OK, f_b defined in the same module.

// b.slang
implementing a;
internal void f_b(); // Defines f_b in module a so they can within the module.
public void publicFunc();

// m.slang
module m;
import a;
void main()
{
    f(); // Error, f is not visible here.
    publicFunc(); // OK.
}
```

`internal` is the default visibility if no other access modifiers are specified.