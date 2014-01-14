# WLC
## WL [Wool] Compiler

WL is a new language meant to be used in a system level context. As such, there will
be no garbage collector, no virtual machine, etc.

## Features

* basic types
* pointers
* structs
* function
* varargs
* importing
* casting
* loops
* basic operators
* declaration/use invarience
* optional semicolons

## Planned features
roughly in order of planned implementation

* switch statements
* array types
* unions
* enums
* vector types
* classes (in an extension language called 'OWL'; equivilent of C to C++)
* generic

## FAQ

### Why no semicolons?
Semicolons are optional. Although I am fine with writing code with semicolons, they are
really a artifact of early compiler development. Furthermore, they are one of the most
common syntax error problems, line based development is more intuitive from a learning
standpoint.

### Why not use C style casting?
C casting requires many brackets. Brackets are annoying. I tried to reduce the number of
brackets required while coding. Also, WL does not require a forward reference for types;
If there is none, it would be ambiguous if the cast statement is a cast, or a bracketed statement
(or alternatively, would require a further lookahead to see if an operator or identifier follows).
Also, I believe it will be easier and more intuitive to learn the idea of casting with the new syntax.

### Why no '->' operator?
the '->' operator is *ugly*. WL uses a '.' instead.

### Why use a caret '^' for pointers?
This was mostly chosen so that a 'power' operator can be added as \*\*. Furthermore, this
also has the benifit of quicker recognition of the operator's usage, due to less mental
confliction with the multiplication operator. It is true that the XOR operator will have
this problem, but it is used much less often.

### Why attach the pointer to the type instead of the identifier?
In WL, a pointer type is declared as 'void^ name' instead of C-style 'void \*name'. It can
be seen that C attaches the pointer declarator to the identifier ('name') instead of the type.
This is because in C, it is functionally attached to the identifier such that a line like:
    int *a, b, c;
declares one int\* a, and two ints b and c. WL's way is much more intuitive, so multiple
declarations like:
    int^ a, b, c
declares 3 int pointers as expected.

### Why no headers?
Headers are nice in that they declare the interface to a module. But unfortnately, when
generics are added, it starts to get more complicated. Anyone who has used generic's in
C++ can see that quickly it gets very messy, with most code ending up in a header. No
longer is a header a declaration of interface, it *is* the source file. It is possible
that WL will 

Although WL currently does not have generics, it is a planned feature.

### Why LLVM?
Using LLVM allows me to focus on the syntax and functionality of the language, and eschew
all optimization and backend logic. This allows me to develop the language much faster and
easier. Furthermore, LLVM's optimization and platform support is likely much better than 
what I would be able to do.

### Why not modify clang?
WL's syntax is different enough that modifying clang would be a significant effort.
Also, I am not familier enough with clang's architecture to do so.
