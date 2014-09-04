/* copyright(C) 2003 H.Kawai (under KL-01). */

#if (!defined(MATH_H))

#define MATH_H	1

#if (defined(__cplusplus))
	extern "C" {
#endif

double sin(double);
double cos(double);
double sqrt(double);
double ldexp(double x, int n);
double frexp(double x, int *exp);

extern __inline__ double sin(double x)
{
	double res;
	__asm__ ("fsin" : "=t" (res) : "0" (x));
	return res;
}

extern __inline__ double cos(double x)
{
	double res;
	__asm__ ("fcos" : "=t" (res) : "0" (x));
	return res;
}

extern __inline__ double sqrt(double x)
{
	double res;
	__asm__ ("fsqrt" : "=t" (res) : "0" (x));
	return res;
}

#if (defined(__cplusplus))
	}
#endif

#endif
