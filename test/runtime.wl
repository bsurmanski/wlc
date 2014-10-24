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

struct Interface
{
    void^ self;
    void function()^ vtable;
}
