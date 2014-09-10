/* 告诉C编译器，有一个函数在别的文件里 */

void io_hlt(void);

/* 是函数声明却不用{}，而用;，这表示的意思是：
	函数在别的文件中，你自己找一下 */

void HariMain(void)
{

fin:
	io_hlt(); /* 执行naskfunc.nas中的_io_hlt函数 */
	goto fin;

}
