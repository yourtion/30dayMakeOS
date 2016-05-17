#include "apilib.h"

char buf[150 * 50];

void HariMain(void)
{
	int win;
	win = api_openwin(buf, 150, 50, -1, "hello");
	api_boxfilwin(win,  8, 36, 141, 43, 3); /*黄色*/
	api_putstrwin(win, 28, 28, 0 /*黑色*/, 12, "hello, world");
	api_end();
}
