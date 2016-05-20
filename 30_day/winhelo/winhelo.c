#include "apilib.h"

void HariMain(void)
{
	int win;
	char buf[150 * 50];

	win = api_openwin(buf, 150, 50, -1, "hello");
	for (;;) {
		if (api_getkey(1) == 0x0a) {
			break; /*按下回车键则break; */
		}
	}
	api_end();
}
