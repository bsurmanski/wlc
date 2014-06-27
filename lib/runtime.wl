struct TypeInfo
{
    //char[] name
    void function()[1] functions
}

struct DynamicArray
{
    void ^ptr
    long size
}

class Object
{
    TypeInfo^ typeInfo
    long refcount
}

use "nomangle"
extern nomangle int printf(char ^fmt, ...);

void dyncall_test(){
    printf("dynamic call!\n");
}
