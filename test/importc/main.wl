use "importc"

import(C) "stdio.h"

int main(int argc, char^^ argv) {
    FILE^ file = fopen("test", "r")
    return 0
}
