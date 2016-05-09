void api_putchar(int c);
void api_end(void);

void HariMain(void){
	char a[100];
	a[10] = 'A'; /*这句当然没有问题*/
	api_putchar(a[10]);
	a[102] = 'B'; /*这句就有问题了*/
	api_putchar(a[102]);
	a[123] = 'C'; /*这句也有问题了*/
	api_putchar(a[123]);
	api_end();
}