extern undecorated int printf(char^ fmt, ...);

void myFunction() {
    printf("funcptr called!\n");
}

int main(int argc, char^^ argv) {
    void function() myfunc = myFunction;
    myfunc();
}
