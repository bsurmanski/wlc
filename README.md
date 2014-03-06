# WLC
## WL [Wool] Compiler

*IMPORTANT*: currently the WL compiler is in development. WLC is not yet ready for active usage.
There is likely to be many bugs and missing features.

WL is a new language meant to be used in a system level context. As such, there will
be no garbage collector, no virtual machine, etc.

## Features

* statically typed
* can interface with C (experimental import(C) expression)
* tuples (unnamed structs)
* multiple return values (using tuples)
* inferred types in declarations using the 'var' keyword
* sized array types
* dynamic memory allocation using 'new'/'delete'/'renew' or 'malloc'/'free'

## Implemented Functionality

* basic types
* pointers
* structs
* tuples
* functions
* varargs
* arrays (with associated sizing)
* importing
* casting
* loops (while, for)
* basic operators
* declaration/use invarience
* optional semicolons
* importing C headers, using clang (experimental. currently able to interface with
  functions, variables, basic types (including pointers), structs. unfortunately not
  preprocessor tokens, const and other things)

## Planned Functionality
roughly in order of planned implementation

* switch statements
* unions
* enums
* vector types
* classes (in an extension language called 'OWL'; similar to the idea of C++ from C)
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
C casting requires many parenthises. Parentheses are annoying. I tried to reduce the number of
brackets required while coding. In this sense, there are fewer moments in which you must go
back and add that missing left parenthesis. Also, WL does not require a forward reference for types;
If there is none, it would be ambiguous if the cast statement is a cast, or a bracketed statement
(or alternatively, would require a further lookahead to see if an operator or identifier follows).
Also, I believe it will be easier and more intuitive to learn the idea of casting with the new syntax.
example:

    float a, b, c
    // some declaration of a,b,c here
    int i = int: a + int: b + int: c

alternatively, if a variable is being assigned to an expression, and the whole expression needs
to be casted, the ':=' operator can be used to cast to the type of the left hand side

    int^ myIntPointer = getSomeIntPointer()
    char^ myCharPointer := myIntPointer
    char^ equivilentCharPointer = char^: myIntPointer

the cast assign operator effectively casts a value without the required noise of casting

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
longer is a header a declaration of interface, it *is* the source file. Additionally, 
these day many people just end up using some sort of IDE, which in itself provides an view
of module interfaces.  It is possible that WL will eventually add generated headers, but
that would be somewhere in the future.

Although WL currently does not have generics, it is a planned feature (sometime in the
future).

### How are arrays represented, and why not just use C-style arrays?
In WL, Arrays are represented as a structure containing a pointer and size. The following
C structure is equivilent to an int array in WL:

    struct Array
    {
        int *ptr;
        long size;
    };

When an array is initialized with a size in WL, (like: int[5] myarray), the 'ptr' member 
is set to a stack allocated buffer of the appropriate size, and the 'size' member is set
to the appropriate size. This can be done statically. Once an array is created, the 'ptr'
and 'size' member can be accessed identically as if the arrays was a struct (with the dot
operator).

In C, arrays are represented as pointers. Often when using an array, a size is required.
As such, in C, a size variable must be passed around seperately. If a simple C style array
is required, you can just use a pointer. (this leads to a problem in structs, I would like
to change this eventually)

### How do I import C files?
use the 'import(C) expression'

    import(C) "stdio.h"

an exact path to the the file may be required. 
this feature is experimental and could change in syntax and semantics.
currently, const or volatile declarations fail to convert correctly and are ignored.

### Why LLVM?
Using LLVM allows me to focus on the syntax and functionality of the language, and eschew
all optimization and backend logic. This allows me to develop the language much faster and
easier. Furthermore, LLVM's optimization and platform support is likely much better than 
what I would ever be able to do within reasonable contraints.

### Why not modify clang?
WL's syntax is different enough that modifying clang would be a significant effort.
Also, I am not familier enough with clang's architecture to do so.

## Future

### 'Use' statement
use statements will enable specific compiler extensions, or modify language syntax. Use
statements will have a syntax of

    use extensionName

extensions can have various effects ranging from syntactical preference (substituting
symbols), to declaring new keywords, types, and behaviour. 

extensions will only affect the module they are declared in. 'use' extensions should not
destructively modify the behaviour of the compiler, and Modules with 'use' extensions
should be able to interface with those without.

standard libraries will not include any 'use' statements.

see 'Ideas' for some examples of possible 'use' extensions.

There may be a set of extensions that are required for a 'conforming' compiler, and a 'embedded' 
standard can be created that does not have the extensions.

### Tail call optimization
self explanitory, avoids stack overflows, makes deep recursive function faster

## Ideas (Not necessarily future additions)

### General Things
structs are anologous to C structs. packing and allignment included

for packed and alligned structs use syntax like struct(packed, allign(4)) { ... }
    this way keywords are conditional

go-like dynamic interfaces

a 'go' keyword that creates a new thread from a function call. (note: it would be a full
thread, not a coroutine)

classes are allowed to reorganize members for proper packing

long cast with :: for statements like a = int:: b + c
    or maybe just have a cast assign a := b + c

### import(CPP)
an equivilent to the import(C) statement, but for CPP files. may only be supported in
'OWL'

### "use objects"
OWL, the object oriented extension to WL, may be added as a simple 'use' extensions that
enables various other features. one idea is to have a "use owl" meta extension that
enables various other extensions like objects, interfaces, vector types.  Maybe a program
will be added (lets call it 'owlc') that runs wlc with the OWL extensions enabled in all
modules by default. similar to the idea of g++ to gcc.

### "use cptr"
cptr extension that modifies the compiler to use '\*' as the pointer specifier symbol (for
fans of traditional C syntax)

### "use semicolon"
semicolons are required to terminate statements. (for fans of traditional C)

### "use cderef"
extensions that forces pointer dereferencing to use the '->' symbol. (for fans of
traditional C syntax)

### "use cbool"
extension that forces boolean keywords 'and', 'or' and 'not' to instead use the C symbols
'&&', '||', and '!'

### "use clike"
meta extension that enables all c-like extensions (cptr, semicolon, cderef, cbool, etc)

### "use restrict"
forbids pointer aliasing, all pointers will be 'restrict' by default. This will allow many
optimization opportunities, but is not enabled by default due to subtilties of behaviour
involving restricting pointer aliasing.

### "use count"
reference counting on classes created within this module

### "use gc"
garbage collection on classes created within this module

### "use threadlocal"
variables in scope will be threadlocal by default. maybe have a keyword 'threadshared'
(maybe 'tshared' or 'shared') to allow a variable to then default to share between threads

### "use zero"
zero out all variables on allocation


### embedded code
allow for embedding different languages with an 'embed' keyword. for example:

    int wlInt = 5;
    embed(C) {
        int cInt = wlInt + 5;
    }
    wlInt += cInt

This example would allow an embedded C statement within a WL scope. This would
also extend to inlined ASM, using embed(ASM). It would need to be considered 
if variables would leak across 'embed' scopes

### Tuples
Due to limitations of static typing, tuples will be more like unnamed structs than
true tuples. This means that indexing can only be done with static integers. This 
is because in a statement like

    [int, char^] myTuple 
    ...
    var myvar = myTuple[i]

it is imposible to determine the type required of 'v', either int or char^.
It may be possible if all types in the tuple are of a uniform type:

    [int, int, int] myIntTuple
    ...
    int myvar = myIntTuple[i]

or if all members of the tuple are convertible to the destination type:

    [char, int, float] myNumberTuple
    ...
    float mydest myNumberTuple[i]

This last case may be exceedingly difficult, to calculate the correct
tuple memory location of the member, and convert to the correct destination
type at runtime.

#### Tuple Syntax

A comma can denote a tuple type, and a tuple value

    int,int,float myTuple
    myTuple = 5, 1, 2.3

alternativly, tuple declarations can be surrounded by [ ]

    [int,int,float] myTuple
    myTuple = [5, 1, 2.3]

Tuples can be implicitly converted to a struct with an identical signature. 

    struct MyStruct
    { 
        int i
        float j
    }

    ...

    [int, float] myTuple = [1, 2.2]
    MyStrct st = myTuple

this would allow compound struct declarations to use the same syntax as tuples.

    MyStruct s = [1, 2, 3]
    [int,int,int] i = [1, 2, 3]

#### Tuple auto unwrap
if a tuple is passed as a function argument, it will automatically be unwrapped to
its components.

    int myFunction(int i, int j, float f);

    ...

    [int,int,float] treble = [1, 2, 1.1]
    myFunction(treble)

    [int,int] pair = [1, 2]
    myFunction(pair, 5.5)

this may also mean that having a tuple as a function argument would simply be syntactic
sugar for auto wrapping on function entry. This would allow a function expecting a tuple
to be called with individual values of the underlying tuple member types.

    void myTupleFunction([int, int] someTuple);

    ...

    myTupleFunction(1, 2)

#### Tuple return
multiple values can be returned in a tuple.

    [int,int] tupleReturner()
    {
        return [1,2]; 
    }

    ...

    [int,int] pair = tuplerReturner()

#### Tuple unpack
Tuples can be unpacked onto multiple variables

    [int, int] pair = [1,2]
    int a
    int b
    a,b = pair

syntax needs to be considered for the assignment...

### Generic operator
Generics can be denoted using the '!' operator.
Generics can use tuples as it's generic parameter, if more than one member is required

    SomeClass!long myfirstvar
    MyClass![int,float] mysecondvar

### Vector Types
Vector and matrix types will be builtin.
