extern nomangle int printf(char ^c, ...);

int main(int argc, char^^ argv) {
    int[] newarr = new int[5]
    printf("%x\n", newarr.ptr);
    delete newarr
}
