undecorated int printf(char^ fmt, ...);

class Animal {
    void speak() {}

    /*
    ~this() {
        printf("deleting an animal\n")
    }*/
}

class Dog : Animal {
    void speak() {
        printf("WOOF\n")     
    }

    this() {
        printf("creating a dog\n")
    }

}

class Human : Animal {
    void speak() {
        printf("HELLO\n")    
    }

    this() {
        printf("creating a human\n")
    }
}

int main(int argc, char^^ argv) {
    Animal animal1 = new Dog()
    Animal animal2 = new Human()

    animal1.speak()
    animal2.speak()
    return 0
}

