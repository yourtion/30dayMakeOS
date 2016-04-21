/* FIFO */

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size, int *buf)
/* FIFO缓冲区的初始化*/
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /*空*/
	fifo->flags = 0;
	fifo->p = 0; /*写入位置*/
	fifo->q = 0; /*读取位置*/
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/*给FIFO发送数据并储存在FIFO中*/
{
	if (fifo->free == 0) {
		/*没有空余空间，溢出*/
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)
/*从FIFO取得一个数据*/
{
	int data;
	if (fifo->free == fifo->size) {
	/*当缓冲区为空的情况下返回-1*/
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo32_status(struct FIFO32 *fifo)
/*报告已经存储了多少数据*/
{
	return fifo->size - fifo->free;
}
