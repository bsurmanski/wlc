import "vec.wl"

extern undecorated int printf(char^ fmt, ...);

int main(int argc, char^^ argv) {
    vec4 a = vec4(1,2,3,4);
    vec4 b = vec4(2.5,3,4,5);
    vec4 c = a.add(b)
    printf("%f %f %f %f\n", c.x(), c.y(), c.z(), c.w())
    return 0
}
