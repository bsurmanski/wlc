#Why?

This document is meant to contain insight into design choices. Like a FAQ for
those interested in language design.


## Why is the typeinfo member in the vtable? why not instead create a TypeInfo
## structure that has a flexible array as it's last member?

Because (using LLVM) you cannot store in a flexible array statically. It would
need to be loaded at runtime. Alternatively, I could have used a pointer within
a TypeInfo which redirects to the vtable. But, I it is prefereable to reduce the
number of pointer redirects.

## Why no semicolons?
Semicolons are optional. Although I am fine with writing code with semicolons, they are
really a artifact of early compiler development. Furthermore, they are one of the most
common syntax error problems, line based development is more intuitive from a learning
standpoint.

## Why is a semicolon required after an external function declarations?
In WL, functions still need to be declared before use (somewhere, not strictly above. can be
declared at the bottom of the file). Usually in WL semicolons are not required, but in
this context, a semicolon represents an empty function body. This means the function
is declared somewhere else. If no semicolon was present, then WL will expect a statement
to follow as the function body. example:
    
    // printf will be linked in with libc, but must be declared to be used
    int printf(char^ fmt, ...);

## Why not use C style casting?
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

the cast assign operator effectively casts a value without the required noise of casting.

Another addition added to help reduce casting noise is the 'infix' cast. This style of casting
is used for long 'dot expression' chains. In a long chain of member references, 
it is often necessary to cast one of the members to another type. Due to the nature of 
cast expressions, the cast type will end up at the beginning of the expression, far away
from the relevent variable. And often this requires exuberant brackets to disambiguate the
member which requires casting. This often creates confusing expressions which requires the
excessive eye-pingponging to understand what is being applied where. WL attempts to reduce
ambiguity in long dereferencing chains by adding an infix cast to keep the casted member 
close to the casted type. Heres an example:

    MyClass bar = getABar();
    int foo1 = (Biz: bar.baz.buzz).value
    int foo2 = bar.baz. Biz: buzz .value

compare the two equivilent casts in the assignment of foo1, and foo2. the second of which
can be read left to right without the need to look back to the beginning of the expression. 

## Why are braces optional for a function body?
this allows quick oneliner functions to be easy to write and read. This also 
falls into the expectation presented by the conditional and loop statments (if,while,etc).
This makes the language more intuitive. example:

    int addFive(int n) return n + 5


## Why use a caret '^' for pointers?
This was mostly chosen so that a 'power' operator can be added as \*\*. Furthermore, this
also has the benifit of quicker recognition of the operator's usage, due to less mental
confliction with the multiplication operator. It is true that the XOR operator will have
this problem, but it is used much less often.


## Why attach the pointer to the type instead of the identifier?
In WL, a pointer type is declared as 'void^ name' instead of C-style 'void \*name'. It can
be seen that C attaches the pointer declarator to the identifier ('name') instead of the type.
This is because in C, it is functionally attached to the identifier such that a line like:
    int *a, b, c;
declares one int\* a, and two ints b and c. WL's way is much more intuitive, so multiple
declarations like:
    int^ a, b, c
declares 3 int pointers as expected.

## Why no '->' operator?
the '->' operator is *ugly*. WL uses a '.' instead. example:

    mystruct^ st = new_mystruct()
    st.member = 5

## Why no headers?
Headers are nice in that they declare the interface to a module. But unfortnately, when
generics are added, it starts to get more complicated. Anyone who has used generic's in
C++ can see that quickly it gets very messy, with most code ending up in a header. No
longer is a header a declaration of interface, it *is* the source file. Additionally, 
these day many people just end up using some sort of IDE, which in itself provides an view
of module interfaces.  It is possible that WL will eventually add generated headers, but
that would be somewhere in the future.

Although WL currently does not have generics, it is a planned feature (sometime in the
future).

## How are arrays represented, and why not just use C-style arrays?
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

## How do I import C files?
use the 'import(C) expression'

    import(C) "stdio.h"

an exact path to the the file may be required. 
this feature is experimental and could change in syntax and semantics.
currently, const or volatile declarations fail to convert correctly and are ignored.

## Why LLVM?
Using LLVM allows me to focus on the syntax and functionality of the language, and eschew
all optimization and backend logic. This allows me to develop the language much faster and
easier. Furthermore, LLVM's optimization and platform support is likely much better than 
what I would ever be able to do within reasonable contraints.

## Why not modify clang?
WL's syntax is different enough that modifying clang would be a significant effort.
Also, I am not familier enough with clang's architecture to do so.

## Why is a dot required to reference a class member?
In most object oriented languages, you may simply name the member and
it will be within scope. In WL, instead you *must* reference members
by indexing the this identifier. As a shortcut, a unary dot is equivilent.

for example:
    class MyClass {
        int member

        int myFunc(){
            return member // Bad!
        }

        int myFunc2(){
            return this.member // Good
        }

        int myFunc3(){
            return .member // Equivilent to this.member, preferable for suscinctness
        }

    }

The reason this is done is to make things more readible.
It is sometimes difficult to tell where a variable is defined
when reading someone else's code, or your own code from long in the past. 
This helps the reader of the code know that the member is contained in the class.
Its a minor syntax addition that should help readibility. Of course, some 
people would rather WL to act like other languages they've used. For this,
there is a 'use' extension

    use "implicit_this"

With "implicit\_this" enabled, no 'this', nor unary dot is required to
reference class members.
