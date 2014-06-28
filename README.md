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
* switch statements
* unions

## Planned Functionality
roughly in order of planned implementation

* classes (WIP, maybe in an extension language called 'OWL'; similar to the idea of C++ from C)
* enums
* vector types
* generic


## Features

### 'Use' statement
use statements enable specific compiler extensions, or modify language syntax. 
When complete, use statements will likely have a syntax of

    use extensionName

Currently, they are enabled by

    use "extensionName"

extensions can have various effects ranging from syntactical preference (substituting
symbols), to declaring new keywords, types, and behaviour. 

extensions only affect the module they are declared in. 'use' extensions should not
destructively modify the behaviour of the compiler, and Modules with 'use' extensions
should be able to interface with those without.

standard libraries will not include any 'use' statements.

see 'Ideas' for some examples of possible 'use' extensions.

There may be a set of extensions that are required for a 'conforming' compiler, and a 'embedded' 
standard can be created that does not have the extensions.

#### Current Use Extensions

##### importc
Used to enable the 'import(C)' construct, which allow importing symbols from a
C header

##### implicit\_this
Allows class members to be referenced without indexing 'this' or using a
unary dot operator

with implicit this:

    use "implicit_this"

    class MyClass {
        int member

        int myFunc(){
            return member
        }
    }

without:

    use "implicit_this"

    class MyClass {
        int member

        int myFunc(){
            return .member
        }
    }


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

tuple declarations will be denoted by surrounding [ ]

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
    [a,b] = pair

All members of the left hand tuple must be LValues.

## Future

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
