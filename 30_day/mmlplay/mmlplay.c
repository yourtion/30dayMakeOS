#include "apilib.h"

#include <string.h>	/* strlen */

int strtol(char *s, char **endp, int base);	 /*标准函数<stdlib.h> */

void waittimer(int timer, int time);
void end(char *s);

void HariMain(void)
{
	char winbuf[256 * 112], txtbuf[100 * 1024];
	char s[32], *p, *r;
	int win, timer, i, j, t = 120, l = 192 / 4, o = 4, q = 7, note_old = 0;

	/*音号与频率（mHz）的对照表*/
	/*例如，04A为440Hz，即440000 */
	/*第16八度的A为1802240Hz，即1802240000 */
	/*以下为第16八度的列表（C～B） */
	static int tonetable[12] = {
		1071618315, 1135340056, 1202850889, 1274376125, 1350154473, 1430438836,
		1515497155, 1605613306, 1701088041, 1802240000, 1909406767, 2022946002
	};
	static int notetable[7] = { +9, +11, +0 /* C */, +2, +4, +5, +7 };

	/*命令行解析*/
	api_cmdline(s, 30);
	for (p = s; *p > ' '; p++) { }	/*一直读到空格为止*/
	for (; *p == ' '; p++) { }	/*跳过空格*/
	i = strlen(p);
	if (i > 12) {
file_error:
		end("file open error.\n");
	}
	if (i == 0) {
		end(0);
	}

	/*准备窗口*/
	win = api_openwin(winbuf, 256, 112, -1, "mmlplay");
	api_putstrwin(win, 128, 32, 0, i, p);
	api_boxfilwin(win, 8, 60, 247,  76, 7);
	api_boxfilwin(win, 6, 86, 249, 105, 7);

	/*载入文件*/
	i = api_fopen(p);
	if (i == 0) {
		goto file_error;
	}
	j = api_fsize(i, 0);
	if (j >= 100 * 1024) {
		j = 100 * 1024 - 1;
	}
	api_fread(txtbuf, j, i);
	api_fclose(i);
	txtbuf[j] = 0;
	r = txtbuf;
	i = 0; /*通常模式*/
	for (p = txtbuf; *p != 0; p++) {	/*为了方便处理，将注释和空白删去*/
		if (i == 0 && *p > ' ') {	/*不是空格或换行符*/
			if (*p == '/') {
				if (p[1] == '*') {
					i = 1;
				} else if (p[1] == '/') {
					i = 2;
				} else {
					*r = *p;
					if ('a' <= *p && *p <= 'z') {
						*r += 'A' - 'a';	/*将小写字母转换为大写字母*/
					}
					r++;
				}
			} else if (*p == 0x22) {
				*r = *p;
				r++;
				i = 3;
			} else {
				*r = *p;
				r++;
			}
		} else if (i == 1 && *p == '*' && p[1] == '/') {	/*段注释*/
			p++;
			i = 0;
		} else if (i == 2 && *p == 0x0a) {	/*行注释*/
			i = 0;
		} else if (i == 3) {	/*字符串*/
			*r = *p;
			r++;
			if (*p == 0x22) {
				i = 0;
			} else if (*p == '%') {
				p++;
				*r = *p;
				r++;
			}
		}
	}
	*r = 0;

	/*定时器准备*/
	timer = api_alloctimer();
	api_inittimer(timer, 128);

	/*主体*/
	p = txtbuf;
	for (;;) {
		if (('A' <= *p && *p <= 'G') || *p == 'R') {	/*音符、休止符*/
			/*计算频率*/
			if (*p == 'R') {
				i = 0;
				s[0] = 0;
			} else {
				i = o * 12 + notetable[*p - 'A'] + 12;
				s[0] = 'O';
				s[1] = '0' + o;
				s[2] = *p;
				s[3] = ' ';
				s[4] = 0;
			}
			p++;
			if (*p == '+' || *p == '-' || *p == '#') {
				s[3] = *p;
				if (*p == '-') {
					i--;
				} else {
					i++;
				}
				p++;
			}
			if (i != note_old) {
				api_boxfilwin(win + 1, 32, 36, 63, 51, 8);
				if (s[0] != 0) {
					api_putstrwin(win + 1, 32, 36, 10, 4, s);
				}
				api_refreshwin(win, 32, 36, 64, 52);
				if (28 <= note_old && note_old <= 107) {
					api_boxfilwin(win, (note_old - 28) * 3 + 8, 60, (note_old - 28) * 3 + 10,  76, 7);
				}
				if (28 <= i && i <= 107) {
					api_boxfilwin(win, (i - 28) * 3 + 8, 60, (i - 28) * 3 + 10,  76, 4);
				}
				if (s[0] != 0) {
					api_beep(tonetable[i % 12] >> (17 - i / 12));
				} else {
					api_beep(0);
				}
				note_old = i;
			}
			/*音长计算*/
			if ('0' <= *p && *p <= '9') {
				i = 192 / strtol(p, &p, 10);
			} else {
				i = l;
			}
			for (; *p == '.'; ) {
				p++;
				i += i / 2;
			}
			i *= (60 * 100 / 48);
			i /= t;
			if (s[0] != 0 && q < 8 && *p != '&') {
				j = i * q / 8;
				waittimer(timer, j);
				api_boxfilwin(win, 32, 36, 63, 51, 8);
				if (28 <= note_old && note_old <= 107) {
					api_boxfilwin(win, (note_old - 28) * 3 + 8, 60, (note_old - 28) * 3 + 10,  76, 7);
				}
				note_old = 0;
				api_beep(0);
			} else {
				j = 0;
				if (*p == '&') {
					p++;
				}
			}
			waittimer(timer, i - j);
		} else if (*p == '<') {	/*八度-- */
			p++;
			o--;
		} else if (*p == '>') {	/*八度++ */
			p++;
			o++;
		} else if (*p == 'O') {	/*八度指定*/
			o = strtol(p + 1, &p, 10);
		} else if (*p == 'Q') {	/* Q参数指定*/
			q = strtol(p + 1, &p, 10);
		} else if (*p == 'L') { /*默认音长指定*/ 
			l = strtol(p + 1, &p, 10);
			if (l == 0) {
				goto syntax_error;
			}
			l = 192 / l;
			for (; *p == '.'; ) {
				p++;
				l += l / 2;
			}
		} else if (*p == 'T') {	 /*速度指定*/
			t = strtol(p + 1, &p, 10);
		} else if (*p == '$') {	/*扩展命令*/
			if (p[1] == 'K') {	/*卡拉OK命令*/
				p += 2;
				for (; *p != 0x22; p++) {
					if (*p == 0) {
						goto syntax_error;
					}
				}
				p++;
				for (i = 0; i < 32; i++) {
					if (*p == 0) {
						goto syntax_error;
					}
					if (*p == 0x22) {
						break;
					}
					if (*p == '%') {
						s[i] = p[1];
						p += 2;
					} else {
						s[i] = *p;
						p++;
					}
				}
				if (i > 30) {
					end("karaoke too long.\n");
				}
				api_boxfilwin(win + 1, 8, 88, 247, 103, 7);
				s[i] = 0;
				if (i != 0) {
					api_putstrwin(win + 1, 128 - i * 4, 88, 0, i, s);
				}
				api_refreshwin(win, 8, 88, 248, 104);
			}
			for (; *p != ';'; p++) {
				if (*p == 0) {
					goto syntax_error;
				}
			}
			p++;
		} else if (*p == 0) {
			p = txtbuf;
		} else {
syntax_error:
			end("mml syntax error.\n");
		}
	}
}

void waittimer(int timer, int time)
{
	int i;
	api_settimer(timer, time);
	for (;;) {
		i = api_getkey(1);
		if (i == 'Q' || i == 'q') {
			api_beep(0);
			api_end();
		}
		if (i == 128) {
			return;
		}
	}
}

void end(char *s)
{
	if (s != 0) {
		api_putstr0(s);
	}
	api_beep(0);
	api_end();
}
