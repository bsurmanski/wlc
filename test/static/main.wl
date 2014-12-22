undecorated int printf(char^ fmt, ...);

struct MyStruct {
    static int i
    int j

    void dostuff(int r) {
        static int i = 0;
        printf("%d\n", i)
        i = r;
    }
}

void func(int r) {
    static int i = 0;
    printf("%d\n", i)
    i = r;

}

int main(int argc, char^^ argv) {
    func(1)
    func(2)
    MyStruct st
    st.dostuff(1)
    st.dostuff(2)
    return 0
}
