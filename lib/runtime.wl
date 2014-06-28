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
    //TypeInfo^ typeInfo
    void function()^ vtable;
    long refcount
}

use "nomangle"
extern nomangle int printf(char^ fmt, ...);

void dyncall_test(){
    printf("dynamic call!\n");
}
