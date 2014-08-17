# Future

## Tail call optimization
self explanitory, avoids stack overflows, makes deep recursive function faster

## Ideas (Not necessarily future additions!!!)

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

### Generic operator
Generics can be denoted using the '!' operator.
Generics can use tuples as it's generic parameter, if more than one member is required

    SomeClass!long myfirstvar
    MyClass![int,float] mysecondvar

### Vector Types
Vector and matrix types will be builtin.
