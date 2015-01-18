extern undecorated int printf(char^ fmt, ...);

struct ArrayStruct {
    int[16] arr
}

int main(int argc, char^^ argv) {
    ArrayStruct st
    st.arr[0] = 5
}
