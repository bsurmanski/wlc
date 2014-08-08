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

it is imposible to statically resolve a type for 'myvar', which will be either
int or char^ depending on if the index 'i' is 0 or 1.  In the future, it may be
possible if all types in the tuple are of a uniform type:

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

This would also have the benefit of allowing structs to
share a common syntax as tuples in declarations.

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
