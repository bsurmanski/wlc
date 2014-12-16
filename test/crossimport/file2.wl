import "file1.wl"

struct OtherStruct
{
    MyStruct ^sillyStruct
}

void add5(MyStruct ^s)
{
    s.i = s.i + 5
    s.j = s.j + 5
}

OtherStruct buildOtherStruct()
{
    OtherStruct st
    st.sillyStruct = null
    return st
}
