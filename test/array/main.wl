extern undecorated int printf(char^ fmt, ...);
extern undecorated float sinf(float f);

char^[] strings = ["hello", "goodbye", "hithere", "thats all folks"]

int[] vals = [1, 2, 3, 4]
int[5] vals2 = [1,2,3,4,5]

void arrayFunc() {
    int[] arr = [1,2,3]
    for(int i = 0; i < arr.size; i++) {
        printf("arr%d: %d\n", i, arr[i])
    }
    arr[0] = 11
}

int main(int argc, char^^ argv)
{
    int[2] i
    i[0] = 11;
    i[1] = 55;
    double x = sinf(3.14);
    printf("%x\n", i.ptr);
    printf("%d\n", i.size);
    printf("%d\n", i[0]);
    printf("%d\n", i[1]);
    printf("%f\n", x);
    int[] arr
    arr = new int[10]

    int[] funcvals = [3,2,1]

    for(int j = 0; j < funcvals.size; j++) {
        printf("%d\n", funcvals[j])
    }

    printf("%d\n", vals[0])
    vals[0] = 5

    for(int j = 0; j < vals.size; j++) {
        printf("%d\n", vals[j])
    }

    arrayFunc()
    arrayFunc()

    return 0
}
