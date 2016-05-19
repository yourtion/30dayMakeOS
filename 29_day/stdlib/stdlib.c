#include "../apilib.h"
#include "../stdlib.h"

#include <stdio.h>
#include <stdarg.h>

int putchar(int c)
{
	api_putchar(c);
	return c;
}

void exit(int status)
{
	api_end();
}

int printf(char *format, ...)
{
	va_list ap;
	char s[1000];
	int i;

	va_start(ap, format);
	i = vsprintf(s, format, ap);
	api_putstr0(s);
	va_end(ap);
	return i;
}

void *malloc(int size)
{
	char *p = api_malloc(size + 16);
	if (p != 0) {
		*((int *) p) = size;
		p += 16;
	}
	return p;
}

void free(void *p)
{
	char *q = p;
	int size;
	if (q != 0) {
		q -= 16;
		size = *((int *) q);
		api_free(q, size + 16);
	}
	return;
}