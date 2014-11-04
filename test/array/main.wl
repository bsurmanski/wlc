extern undecorated int printf(char^ fmt, ...);
extern undecorated float sinf(float f);

char^[] strings = ["hello", "goodbye", "hithere", "thats all folks"]

int[] vals = [1, 2, 3, 4]
int[5] vals2 = [1,2,3,4,5]

const int SIZE = 5
int[SIZE] sillyArray

void arrayFunc() {
    int[] arr = [1,2,3]
    for(int i = 0; i < arr.size; i++) {
        printf("%d ", arr[i])
    }
    arr[0] = 11
}

int main(int argc, char^^ argv)
{
    printf("static const array size: %d\n", sillyArray.size);
    int[2] i
    i[0] = 11;
    i[1] = 55;
    double x = sinf(3.1415926);
    printf("ptr: %x\n", i.ptr);
    printf("size: %d\n", i.size);
    printf("i[0]: %d\n", i[0]);
    printf("i[1]: %d\n", i[1]);
    printf("sin(PI): %f\n", x);
    int[] arr
    arr = new int[10]

    int[] funcvals = [3,2,1]

    for(int j = 0; j < funcvals.size; j++) {
        printf("FUNCVAL[j] = %d\n", funcvals[j])
    }

    printf("VALS[0] (1?) = %d\n", vals[0])
    vals[0] = 5

    printf("Should print 5 2 3 4: ");
    for(int j = 0; j < vals.size; j++) {
        printf("%d ", vals[j])
    }
    printf("\n");

    printf("should print 1 2 3 twice: ");
    arrayFunc()
    arrayFunc()
    printf("\n");

    return 0
}
