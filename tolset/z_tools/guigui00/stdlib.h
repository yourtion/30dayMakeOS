/* copyright(C) 2003 H.Kawai (under KL-01). */

#if (!defined(STDLIB_H))

#define STDLIB_H	1

#if (defined(__cplusplus))
	extern "C" {
#endif

#include <stddef.h>		/* size_t */

#define	RAND_MAX	0x7fff
#define srand(seed)			(void) (rand_seed = (seed))

int abs(int n);
double atof(const char *s);
int atoi(const char *s);
void qsort(void *base, size_t n, size_t size,
	int (*cmp)(const void *, const void *));
int rand(void);
extern unsigned int rand_seed;
double strtod(const char *s, const char **endp);
long strtol(const char *s, const char **endp, int base);
unsigned long strtoul(const char *s, const char **endp, int base);

void *malloc(unsigned int nbytes);
void free(void *ap);

#if (defined(__cplusplus))
	}
#endif

#endif
