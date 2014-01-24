void^ malloc(long sz);
int printf(char^ fmt, ...);


int main(int argc, char^^ argv)
{
    int[2] i
    i[0] = 11;
    i[1] = 55;
    int ^someptr = i;
    printf("%x\n", i.ptr);
    printf("%d\n", i.size);
    printf("%d\n", i[0]);
    printf("%d\n", i[1]);
    return 0    
}
