import "file1.wl"
import "file2.wl"

int printf(char^fmt, ...);

int main(int argc, char^^ argv)
{
    MyStruct s
    initMyStruct(&s, 1, 2)
    MyStruct^ myst = retStruct()
    add5(&s)
    printf("myStruct: %d, %d\n", s.i, s.j)
    return 0
}
