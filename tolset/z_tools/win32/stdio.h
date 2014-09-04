/* copyright(C) 2003 H.Kawai (under KL-01). */

#if (!defined(STDIO_H))

#define STDIO_H	1

#if (defined(__cplusplus))
	extern "C" {
#endif

#if (!defined(NULL))
	#define NULL	((void *) 0)
#endif

#include <stdarg.h>

/* golibc */
int sprintf(char *s, const char *format, ...);
int vsprintf(char *s, const char *format, va_list arg);

/* w32clibc */
typedef struct {
	unsigned int handle;
	int flags; /* bit0:text/bin, bit1:input-enabe, bit2:output-enable */
		/* bit3:EOF, bit4:ungetc-enable */
	int ungetc;
} FILE;
extern FILE __stdin, __stdout, __stderr;
#define	stdin	(&__stdin)
#define	stdout	(&__stdout)
#define	stderr	(&__stderr)
#define EOF		-1
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

unsigned int fwrite(const void *ptr, unsigned int size, unsigned int nmemb, FILE *stream);
int fputs(const char *s, FILE *stream);
int fprintf(FILE *stream, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list arg);
int puts(const char *s);
int printf(const char *format, ...);
int vprintf(const char *format, va_list arg);
int fclose(FILE *stream);
int fflush(FILE *stream);
FILE *fopen(const char *filename, const char *mode);
unsigned int fread(void *ptr, unsigned int size, unsigned int nobj, FILE *stream);
int fseek(FILE *stream, int offset, int origin);
int ftell(FILE *stream);
int remove(const char *filename);
int fputc(int c, FILE *stream);
void clearerr(FILE *stream);
void rewind(FILE *stream);
int fgetc(FILE *stream);
int feof(FILE *stream);
#define getc		fgetc
#define getchar()	getc(stdin)
#define putchar(c)	fputc(c, stdout)
char *fgets(char *s, int n, FILE *stream);
char *gets(char *s);
int ungetc(int c, FILE *stream);

#if (defined(__cplusplus))
	}
#endif

#endif
