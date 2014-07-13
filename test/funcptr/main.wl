nomangle int printf(char^ fmt, ...);

void update() {
    printf("update called!\n");
}

int main(int argc, char^^ argv) {
    void function() myfunc = update;
    myfunc();
}
