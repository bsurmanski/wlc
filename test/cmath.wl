double M_E      = 2.7182818284590452354 /* e */
double M_LOG2E	= 1.4426950408889634074 /* log_2 e */
double M_LOG10E	= 0.43429448190325182765 /* log_10 e */
double M_LN2    = 0.69314718055994530942 /* log_e 2 */
double M_LN10   = 2.30258509299404568402 /* log_e 10 */
double M_PI		= 3.14159265358979323846 /* pi */
double M_PI_2	= 1.57079632679489661923 /* pi/2 */
double M_PI_4	= 0.78539816339744830962 /* pi/4 */
double M_1_PI	= 0.31830988618379067154 /* 1/pi */
double M_2_PI	= 0.63661977236758134308 /* 2/pi */
double M_2_SQRTPI= 1.12837916709551257390 /* 2/sqrt(pi) */
double M_SQRT2	= 1.41421356237309504880 /* sqrt(2) */
double M_SQRT1_2= 0.70710678118654752440 /* 1/sqrt(2) */

double sin(double x);
double cos(double x);
double tan(double x);
double asin(double x);
double acos(double x);
double atan(double x);
double atan2(double x);
double cosh(double x);
double sinh(double x);
double tanh(double x);
double exp(double x);
double log(double x);
double pow(double base, double exp);
double sqrt(double x);
void sincos(double x, double^ sn, double^ cs);

float sinf(float x);
float cosf(float x);
float tanf(float x);

double ceil(double x);
double floor(double x);
double fmod(double num, double denom);
double round(double x);

double copysign(double x, double y);
float copysignf(float x, float y);

double fmin(double x, double y);
float fminf(float x, float y);

double fabs(double x);
