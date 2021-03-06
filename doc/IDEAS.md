## Structures and classes

structs are anologous to C structs. packing and allignment included

for packed and alligned structs use syntax like struct(packed, allign(4)) { ... }
    this way keywords are conditional

classes are allowed to reorganize members for proper packing

long cast with :: for statements like a = int:: b + c
    or maybe just have a cast assign a := b + c

tuples, being basically dynamically allocated structs

foreach
find something in arrays
sorting
libraries

maybe have a dynamic cast operator? maybe ::
eg MyClass:: value
maybe if needed, can have a reinterpret cast as :::
int::: myfloat

have various different types of import statements. eg: import(string), import(file) or import(include)
have the return value be determined by the import type.

have a 'wlx' utility that will transform the source. eg. transform to-from extensions.

infix casting for chained dot expressions. example: MyClass. MyOtherClass: member.function()

null coelescing operator

ASM can have a syntax similar to other functions: 
    asm [ax, bx] somefunc(rax, eax, al)
    {
        addl %1 %0
        xorl %%eax %%eax
    }

## Switch Statements

eschew 'default' keyword. 'default' should be whatever content comes before
the  first explicit case:

switch(myValue)
{
    print("ERROR, invalid value")
    case 1
        print("I Have one argument")
    case 2
        print("I Have one argument")
}

if myValue is '3', then 'ERROR' will be printed


## Alias Types

new type expression similar to 'go'. types will 'extend' existing types for type safety. 
Conversion, special operators, and specific methods can be added. No members may be added to the body.
'type's will be a compile time and debug info construct only, with no runtime overhead.

    type MyType : int {
        MyType operator+(int o) return this + o;
    }

maybe 'this' isnt quite right, but it seems to fit

may also have an 'implicit' (alt. 'silent') keyword for implicit up conversion

    type MyType : implicit int

in this case, an int may be implicitly converted to MyType in an expression

-- 
may also use the 'implicit' keyword in function declarations with aliased parameters. 

    void myFunction(implicit MyType mt, int i) {}

this function will accept a 'MyType' or the underlying 'int' type

--
maybe also have a range modifier to the type (ala Ada). 

    type MyType = int(0..10)

actually, probably only have the range in the 'extend' syntax.
    
    type MyType : int(0..10)

this way, it will create a new 'type', where the 'equals' will simply alias


Or potentially this could be extended to an assertion expression. only certain valid values are allowed.

    type MyType = int(this % 2)
    type MyType = int(this in 0..10)
    type MyType : int(this in [2, 5, 7]) {
        int doStuff(int i) { return i + this; }
    }

Could potentially use this syntax for casting also

    int i = int(0 .. 10): myValue

not sure what would happen if a range check fails. maybe it simply crashes? assert(false)?
range check only valid during debug builds?

perhaps dont use parens for the condition. This would prevent casting of struct/class types,
due to similarity of syntax to call. potentially disambiguatable, but messy. maybe use
'where' keyword instead

    MyStruct st = MyStruct(this.i == 2): getMyStruct()
    MyStruct st = MyStruct where(this.i == 2): getMyStruct()
    type MyType = int where(this == 5)
    



###
keyword direct/indirect to replace 'final'/'virtual'.
keyword decorated/undecorated to replace 'mangle'
decorated(C), decorated(CPP)
intern keyword for internal linkage. replace 'static' on functions

###
constructors in classes (and maybe in other usertypes).
    use 'this' to specify constructor methods. constructors *must* be explicitly called with
    a call, eg.
    MyClass c = MyClass()
    will run the no arguments constructor
default constructors. uses syntax of "implicit this() {}". If an 'implicit this' is found
in the type, it will be called when creating the type on the stack, and implicitly in all other
constructors (maybe).
The reason for the distinction and distinct syntax 
is so that no argument constructors can be used in structs/etc.

###
codegen type expressions
create a 'type' runtime type

###
Generics/Templates

instead of the whole C++/D/Java idea of template/generics, provide the
same functionality by having static function parameters and currying.

eg.

    void myFunction(static type T, int i) {
        #T myThing = i
    }

    void exprFunc(static expression E) {
        int i = #E * #E
    }

    void function2() {
        int j = 2
        myFunction(int, 5)
        let newFunc = myFunction!(int) // curry without call
        newFunc(10)
        exprFunc(5 * j) // int i = (5*j) * (5*j)
    }

or maybe with 'static' replaced with the '#' symbol to signify generics.

    void myFunction(#type T, int i) {
        #T myThing = i
    }

although this would not handle generic classes...

perhaps can also have following syntax:

class MyClass(T) {
    ...
}

with instanciation as:

MyClass#int classVariable; ...

#######

function/variable attribute which zeros variables after finished.
used for crypto code. eg.

    secure int getMagicKey(int val) {
        char[32] buf
        int ret
        ... // use buf to calcuate ret
        return ret
    }

buf would be zeroed out before return
https://news.ycombinator.com/item?id=8277928
