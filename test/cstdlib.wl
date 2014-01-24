import "cstdio.wl"

int RAND_MAX = 214783647
int EXIT_FAILURE = 1
int EXIT_SUCCESS = 0

double atof(char^ nptr);
int atoi(char^ nptr);
long atol(char^ nptr);
long atoll(char^ nptr);
double strtod(char^ nptr, char^^ endptr);
float strtof(char ^nptr, char^^ endptr);
long strtol(char^ nptr, char^^ endptr, int base);
ulong strtoul(char^ nptr, char^^ endptr, int base);
long strtoq(char^ nptr, char^^ endptr, int base);
ulong strtouq(char^ nptr, char^^ endptr, int base);
char ^initstate(uint seed, char^ statebuf, long statelen);
char ^setstate(char ^statebuf);
int rand();
void srand(uint seed);
int rand_r(uint^ seed);
double drand48();
double erand48(ushort^ xsubi);
long lrand48();
long nrand(ushort^ xsubi);
long mrand48();
long jrand48(ushort^ xsubi);
void srand48(long seedval);
ushort^ seed48(ushort^ seed16v);

void^ malloc(long size);
void^ calloc(long nmemb, long size);
void^ realloc(void^ ptr, long size);
void free(void^ ptr);
void^ alloca(long size);

void abort();
void exit(int status);

int putenv(char^ str);
int setenv(char^ nm, char^ val, int replace);
int unsetenv(char^ nm);
int clearenv();
char^ mktemp(char^ tmplat);
char^ mkdtempt(char^ tmplat);
int system(char^ command);
char^ realpath(char^ nm, char^ resolved);
//void^ bsearch(void^ key, void^ base, long nmemb, long sz
//void^ qsort(void^ base, long nmemb, long sz
int abs(int x);
long labs(long x);
long llabs(long x);

int mblen(char^ s, long n);

void assert(bool cnd, char^ err) 
{
    if(cnd) fputs(err,stderr);
    fflush(stderr);
    abort();
}
