#include "apilib.h"

void HariMain(void)
{
	char *buf;
	int win;

	api_initmalloc();
	buf = api_malloc(150 * 50);
	win = api_openwin(buf, 150, 50, -1, "hello");
	api_boxfilwin(win,  8, 36, 141, 43, 6); /*浅蓝色*/
	api_putstrwin(win, 28, 28, 0 , 12, "hello, world");/*黑色*/
	api_end();
}
