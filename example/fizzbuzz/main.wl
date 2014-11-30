undecorated int printf(char^ fmt, ...);

/*
 * Fizz buzz program. Iterate from i=0 to 100.
 * On multiples of 3, print 'fizz'. 
 * On multiples of 5, print 'buzz'.
 * On multiples of both 3 and 5, print 'fizzbuzz'
 *
 * This does not show off any unique features of OWL, but shows
 * some similarities with C's syntax
 */
int main(int argc, char^^ argv) {
    for(int i = 0; i < 100; i++) {
        printf("%d ", i)
        if(i % 3 == 0 and i % 5 == 0) {
            printf("fizzbuzz!")
        } else if(i % 3 == 0) {
            printf("fizz!")
        } else if(i % 5 == 0) {
            printf("buzz!")
        }
        printf("\n")
    }
}
