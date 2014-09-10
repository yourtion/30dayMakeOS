void io_hlt(void);

void HariMain(void)
{
	int i; /* 声明变量i，i是32位整数 */
	char *p; /* 声明变量p、用于BYTE [...]地址 */

	p = (char *) 0xa0000; /* 地址变量赋值 */

	for (i = 0; i <= 0xffff; i++) {
		*(p + i) = i & 0x0f;
	}

	for (;;) {
		io_hlt();
	}
}
