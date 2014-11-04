struct TypeInfo
{
    char[] name
    void^ vtable;
}

struct DynamicArray
{
    void^ ptr
    long size
}

class Object
{
    void function()^ vtable;
    long refcount
}

// currently not used
undecorated void __wlretain(weak Object o) {
    if(o == null) return
    o.refcount++
}

// currently not used
undecorated void __wlrelease(weak Object o) {
    if(o == null) return
    o.refcount--
    if(o.refcount <= 0) delete o
}


struct Interface
{
    void^ self;
    void function()^ vtable;
}
