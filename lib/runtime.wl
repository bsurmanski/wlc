struct TypeInfo
{
    char[] name
}

struct VTable
{
    void^ typeinfo
    void^[1] functions
    // ... functions extend
}

struct DynamicArray
{
    void ^ptr
    long size
}

void SOMERUNTIMEFUNC()
{
    
}
