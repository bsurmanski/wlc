struct MyStruct
{
    int i
    int j
}

void initMyStruct(MyStruct^ s, int i, int j)
{
    s.i = i
    s.j = j
}

void^ malloc(int sz);

MyStruct^ retStruct()
{
    MyStruct ^st = malloc(20);
    st.i = 11
    st.j = 71
    return st
}
