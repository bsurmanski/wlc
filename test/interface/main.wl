//use "implicit_this"

import "myclass.wl"
import "someface.wl"


void callFunctions(SomeFace face) {
    face.func1()
    face.func2()
    face.func2(2)
}

int main(int argc, char^^ argv)
{
    MyClass cl = new MyClass
    MySpecialClass scl = new MySpecialClass
    SomeFace face1 = cl
    SomeFace face2 = scl
    callFunctions(face1)
    callFunctions(face2)
    return 0
}
