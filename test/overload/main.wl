
class OverloadClass {
    int i

    void classy() {
        printf("i am so classy\n")
    }

    void classy(int i) {
        printf("i am %d times as classy\n", i)
    }
}

void func(int i) {
    printf("overload x1\n")
}

int func(int i, int j) {
    printf("overload x2\n")
    return 5
}

int main(int argc, char^^ argv)
{
    func(5)
    func(10, 11);
    OverloadClass cl = new OverloadClass()
    cl.classy()
    cl.classy(5)
}
