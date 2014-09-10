void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void)
{
	int i; /* 声明变量，i是32位整数 */

	for (i = 0xa0000; i <= 0xaffff; i++) {
		write_mem8(i, 15); /* MOV BYTE [i],15 */
	}

	for (;;) {
		io_hlt();
	}
}
