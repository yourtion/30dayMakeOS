void io_hlt(void);

void HariMain(void)
{
	int i; /* 声明变量i，i是32位整数 */
	char *p; /* 声明变量p、用于BYTE [...]地址 */

	for (i = 0xa0000; i <= 0xaffff; i++) {
		p = (char *)i;
		*p = i & 0x0f;
		/* 替代write_mem8(i, i & 0x0f); */
	}

	for (;;) {
		io_hlt();
	}
}
