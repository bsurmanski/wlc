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

* importing C headers (interfacing with clang's AST)
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

### Why is a semicolon required after an external function declarations?
In WL, functions still need to be declared before use (somewhere, not strictly above. can be
declared at the bottom of the file). Usually in WL semicolons are not required, but in
this context, a semicolon represents an empty function body. This means the function
is declared somewhere else. If no semicolon was present, then WL will expect a statement
to follow as the function body. example:
    
    // printf will be linked in with libc, but must be declared to be used
    int printf(char^ fmt, ...);

### Why not use C style casting?
C casting requires many brackets. Brackets are annoying. I tried to reduce the number of
brackets required while coding. Also, WL does not require a forward reference for types;
If there is none, it would be ambiguous if the cast statement is a cast, or a bracketed statement
(or alternatively, would require a further lookahead to see if an operator or identifier follows).
Also, I believe it will be easier and more intuitive to learn the idea of casting with the new syntax.
example:

    float a, b, c
    // some declaration of a,b,c here
    int i = int: a + int: b + int: c

### Why are braces optional for a function body?
this allows quick oneliner functions to be easy to write and read. This also 
falls into the expectation presented by the conditional and loop statments (if,while,etc).
This makes the language more intuitive. example:

    int addFive(int n) return n + 5


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

### Why no '->' operator?
the '->' operator is *ugly*. WL uses a '.' instead. example:

    mystruct^ st = new_mystruct()
    st.member = 5

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
