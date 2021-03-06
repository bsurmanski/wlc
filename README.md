# WLC
## OWL Compiler

*IMPORTANT*: currently the OWL compiler is in development. WLC is not yet ready for active usage.
There is likely to be many bugs and missing features. Additionally, development
may not be stable. It is recommended to download a tagged release version.

OWL is a new language meant to be used in a system level context. As such, there will
be no garbage collector, no virtual machine, etc.

## Language Summary
The OWL language is an object-oriented system level programming language without
a garbage collector. It is intended to be used in real time applications where
low overhead is important. The basic design and syntax follows that of C,
only deviating where a given choice no longer makes sense in a modern context or
where the compromise between pleasing code outweighs any slight compiler
complexity incurred. 

The most obvious deviant features are classes, pointer syntax, cast syntax,
tuples, importing, function overloading, uniform function call syntax and
optional semicolons.

The language can currently interface with C by importing C headers, as an
extension (see *Current 'Use' Extension*).

The current feature set rivals that of C, with some additional primitive types
(mainly classes, tuples, and dynamic arrays).

There is also a planned subset of OWL called WL, without object oriented
features.

## Why
There are already many languages in existence, so why create a new one? No
language I could find fits the niche I am looking for. I would like a compiled, low-level,
object oriented language without a garbage collector. Currently, the languages
that fit that niche is limited to C++ and Fortran. C++ is a powerful language,
but it has many strange pitfalls. C++ was designed in a different time, and it's
starting to show. 'Modern C++' attempts to reconcile this, but the problem is at
that the core design is outdated and each new feature is patching up an
existing feature. For example, the preprocessor, inherited from C, no longer
makes sense within modern software engineering practices. Templates were then
added and they solve the same fundamental problem. 

OWL wishes to rethink what is required of an object-oriented system level
programming language, while still remaining familiar, intuitive and powerful. 

From a personal point of view, The design and implementation of OWL is an
exercise intended to increase my understanding of how modern languages work and
increase my knowledge of modern compiler tooling (specifically LLVM).

## The Name
The name is mostly meaningless. I wanted a short 2-3 letter name that was simple
and memorable. 

If you wish, the name may also stand for 'Objective WL', where WL may stand for
'Without-object Language'.

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
* declaration/use invariance
* optional semicolons
* importing C headers, using clang (experimental. Currently able to interface
  with functions, variables, basic types (including pointers), structs.
  Unfortunately, some complex preprocessor tokens, const and other things are
  not supported)
* switch statements
* unions
* classes
* interfaces
* uniform function call syntax

## Planned Functionality
roughly in order of planned implementation

* enums
* vector types
* generics

## Features

### Classes
Classes are used to encapsulate functionality and state into a single object. On
top of this, classes provide a way of polymorphism. Polymorphism can be used to
generalize algorithms by encapsulating implementation specific functionality
within the class. In OWL, classes are reference types, and are reference counted.

Below is a basic example of class polymorphism in OWL:

    extern undecorated int printf(char^ fmt, ...);

    class Animal {
        int age

        this() {
            .age = 0
        }

        void speak() {
            // do nothing, implementation unknown
        }

        int lifespan() {
            return 0
        }

        bool isElderly() { 
            return .age > .lifespan()
        }
    }

    class Dog : Animal {
        void speak() {
            printf("woof!\n")
        }

        int lifespan() {
            return 20
        }
    }

    class Human : Animal {
        void speak() {
            printf("Hello!\n")
        }

        int lifespan() {
            return 80
        }
    }

    int main(int argc, char^^ argv) {
        Animal human = new Human()
        Animal dog = new Dog()
        human.speak()
        dog.speak()
        return 0
    }

The above example shows off member access syntax, virtual functions, and
constructor syntax.

Classes in OWL only allow single inheritance. Apart from the explicit members,
classes are implemented with an implicit virtual table pointer and 64-bit
reference count. This implies that the benefit of polymorphism in OWL comes at
the cost of 128-bit overhead in contrast to traditional structs.

#### Reference Counting
In OWL, Classes are special among types in that they are reference counted. A
reference is retained on assignment, and released when overwritten or on scope
exit.

    void myFunction() {
        Animal human = new Human()
        ...
        // human implicitly released on scope exit
    }

There are also 'weak' references. Weak references do not retain or release on
assignment. This provides a way to prevent cyclic references.

    Animal human = new Human()
    weak Animal myAnimal = human // no retain for myAnimal

### Interfaces
Interfaces allow a convenient way for dynamic method dispatch without
inheritence. This allows a programmer to create an abstract type that defines a
behaviour. One or more implementations of this behaviour can be created. When an
interface is used, the calling code does not need to worry about the
implementation.


    // provide an interface to work with some input
    inteface InputInterface {
        long size();
        long read(void^ buf, long sz, long nmem);
    }

    // may implicitly cast to InputInterface
    // represents using a file on disk as an input
    class File {
        FILE^ file
        long sz

        long size() {
            ...
        }

        long read(void^ buf, long sz, long nmem) {
            ...
        }
    }

    // may implicitly cast to InputInterface
    // represents using a string in memory as an input
    class StringFile {
        long size() {
            ...
        }

        long read() {
            ...
        }
    }

    ...

    Mesh loadMesh(InputInterface io) {
        // use methods defined in input interface
        // does not matter if input is from memory or disk
        ...
    }

    void main() {
        // loads file during runtime
        Mesh a = loadMesh(new File("file.msh"))

        // packs file into memory at compile time
        // then loads mesh from memory during runtime
        Mesh b = loadMesh(new StringFile(pack "file.msh"))
    }

### Arrays
arrays have an associated size in OWL. Arrays can either be statically sized, or
dynamically sized. Statically sized arrays cannot be resized but an explicit
size variable does not need to be stored. Static arrays are implemented as a
simple pointer. Dynamically sized arrays are stored with a pointer to the first
element as well as a size of the array. This implies that dynamic arrays have an
overhead of an additional 64-bit 'size' member over the statically typed
arrays.

    // stored as 5 elements of integers
    int[5] staticArray = [1,2,3,4,5]   

    // stored as pointer to array of integers, and a 'long' size variable
    int[] dynamicArray = [1,2,3,4,5,6]  

    printf("my static array has %d elements", staticArray.size)
    printf("my dynamic array has %d elements", dynamicArray.size)

    dynamicArray[0] = 50
    staticArray[0] = 10

It can be seen that static arrays and dynamic arrays are used identically. The
difference comes when storing in structs or passing to, and returning from
functions

    struct MyStruct {
        char[32] staticArray     // exactly 32 bytes in MyStruct
        char[] dynamicArray      // exactly 16 bytes in MyStruct (8 for pointer, 8 for array size)
    }

### Function Overloading and Default Parameters
Functions can be overloaded in OWL by parameter type. Additionally, function
parameters can be given default values. If some arguments are not provided, then
the default value will be used.

    int sub(int i) {
        return -i
    }

    int sub(int i, int j) {
        return i - j
    }

    int add(int i = 0, int j = 0) {
        return i + j
    }

    sub(5)      // calls first version of 'sub'
    sub(5,6)    // calls second version of 'sub'
    add()       // valid, returns 0 (i=0, j=0)
    add(1)      // valid, returns 1 (i=1, j=0)
    add(1, 2)   // valid, return 3  (i=1, j=2)

### Tuples
Tuple provide a way to combine multiple data types into a single record without
providing an explicit type name.  Due to limitations of static typing, tuples
will be more like unnamed structs than true tuples of dynamic languages. This
means that indexing can only be done with static integers. This is because in a
statement like

    [int, char^] myTuple 
    ...
    var myvar = myTuple[i]

It is impossible to statically resolve a type for 'myvar', which will be either
int or char^ depending on if the index 'i' is 0 or 1.  In the future, it may be
possible if all types in the tuple are of a uniform type:

    [int, int, int] myIntTuple
    ...
    int myvar = myIntTuple[i]

Or if all members of the tuple are convertible to the destination type:

    [char, int, float] myNumberTuple
    ...
    float mydest myNumberTuple[i]

This last case may be exceedingly difficult, to calculate the correct
tuple memory location of the member, and convert to the correct destination
type at runtime.

#### Tuple Syntax

Tuple declarations will be denoted by surrounding [ ]

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

### Switch Statements
Switch statements in OWL provide a way to select among many outcomes depending on
the value of a variable. Cases can be combined to allow many values to map to
the same outcome. When a subsequent case is encountered, control immediately
exits the switch. There is no fall-through. Currently, there is no 'default'
statement, the default outcome is that which proceeds any other provided cases.

    switch(days) {
        printf("very slow")

        case 0
            printf("very fast")
        case 1,2,3
            printf("pretty standard")
    }

### Modules
Each code file is a module. Symbols defined in a module may be used in another
module by using an 'import' statement.

    import "myModule.wl"

Importing defines the symbol of the imported module to be used within the
importing module. Modules will only be evaluated once, regardless of the number
of other modules which import it.

The current module handling is fairly simple and likely to change in the future.

### 'Use' statement
use statements enable specific compiler extensions, or modify language syntax. 
When complete, use statements will likely have a syntax of

    use extensionName

Currently, they are enabled by

    use "extensionName"

Extensions can have various effects ranging from syntactical preference (substituting
symbols), to declaring new keywords, types, and behaviour. 

Extensions only affect the module they are declared in. 'use' extensions should not
destructively modify the behaviour of the compiler, and Modules with 'use' extensions
should be able to interface with those without.

Standard libraries will not include any 'use' statements.

See the 'Ideas' document for some examples of possible 'use' extensions.

There may be a set of extensions that are required for a 'conforming' compiler, and a 'embedded' 
standard can be created that does not have the extensions.

#### Current 'Use' Extensions

##### importc
Used to enable the 'import(C)' construct, which allow importing symbols from a
C header. import(C) allows most C symbols to be imported and used. This includes
function declarations, global variables, macro constants, and typedefs.

While import(C) works for most cases, unfortunately some more complicated
preprocessor macros can not be used. Currently functional macros and macros that
contain complex expressions cannot be parsed and are ignored.

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

## Building And Installing

### Linux
For linux, a Makefile is provided. WLC requries the following to build:

* LLVM (3.4 - 3.6)
* g++
* zlib
* curses (for LLVM)
* libdl

after running 'make', 'make install' may be run. The Makefile is hardcoded
to install wlc to /usr/local/bin, any OWL libraries to /usr/local/include/wl,
and to copy the vim syntax file to ~/.vim/syntax.

### Windows
A Visual Studios project file is provided in the *win32* directory. For Windows,
the libclang source must be availible to compile.

For more information, view the WIN32\_README.md file in the *win32* directory.

A Windows Binary is available for download [here](https://www.amazon.ca/clouddrive/share/KswlO3usuwAQmjlWw_dpz-h_2ccvPuzdcba0FK-BtEU) (commit "dc366a17" Feb 13 2015)

### OSX
WLC is likely to compile on OSX using the provided Makefile with minor
modification. In the past, I have successfully compiled WLC on OSX, and ran a
simple test program. Unfortunately, WLC has not been tested or fully ported to
OSX. 

In the current state, WLC should be able to compile pure OWL code, but importing
C headers is almost certain to fail. 

A proper OSX port can be created on request.

## Contact
For more information, contact me at:

email: b (dot) surmanski (at) gmail (dot) com

twitter: bsurmanski

