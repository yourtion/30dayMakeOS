#include "bootpack.h"
#include <setjmp.h>
#include <string.h>
#define NULL		0

typedef unsigned char UCHAR;
typedef unsigned int UINT32;
typedef UINT32 tek_TPRB;

static int tek_decode1(int siz, UCHAR *p, UCHAR *q);
static int tek_decode2(int siz, UCHAR *p, UCHAR *q);
static int tek_decode5(int siz, UCHAR *p, UCHAR *q);

static unsigned int tek_getnum_s7s(UCHAR **pp)
{
	unsigned int s = 0;
	UCHAR *p = *pp;
	do {
		s = s << 7 | *p++;
	} while ((s & 1) == 0);
	s >>= 1;
	*pp = p;
	return s;
}

int tek_getsize(unsigned char *p)
{
	static char header[15] = {
		0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x4f, 0x53, 0x41, 0x53, 0x4b, 0x43, 0x4d, 0x50
	};
	int size = -1;
	if (memcmp(p + 1, header, 15) == 0 && (*p == 0x83 || *p == 0x85 || *p == 0x89)) {
		p += 16;
		size = tek_getnum_s7s(&p);
	}
	return size;
}	  /* （注）memcmp和strncmp差不多，这个函数忽略字符串中的0并一直比较到指定的15个字符为止*/

int tek_decomp(unsigned char *p, char *q, int size)
{
	int err = -1;
	if (*p == 0x83) {
		err = tek_decode1(size, p, q);
	} else if (*p == 0x85) {
		err = tek_decode2(size, p, q);
	} else if (*p == 0x89) {
		err = tek_decode5(size, p, q);
	}
	if (err != 0) {
		return -1; /*失败*/
	}
	return 0;	/*成功*/
}

static int tek_lzrestore_stk1(int srcsiz, UCHAR *src, int outsiz, UCHAR *q)
{
	int by, lz, cp, ds;
	UCHAR *q1 = q + outsiz, *s7ptr = src, *q0 = q;
	do {
		if ((by = (lz = *s7ptr++) & 0x0f) == 0)
			by = tek_getnum_s7s(&s7ptr);
		if ((lz >>= 4) == 0)
			lz = tek_getnum_s7s(&s7ptr);
		do {
			*q++ = *s7ptr++;
		} while (--by);
		if (q >= q1)
			break;
		do {
			ds = (cp = *s7ptr++) & 0x0f;
			if ((ds & 1) == 0) {
				do {
					ds = ds << 7 | *s7ptr++;
				} while ((ds & 1) == 0);
			}
			ds = ~(ds >> 1);
			if ((cp >>= 4) == 0) {
				do {
					cp = cp << 7 | *s7ptr++;
				} while ((cp & 1) == 0);
				cp >>= 1;
			} 
			cp++;
			if (q + ds < q0)
				goto err;
			if (q + cp > q1)
				cp = q1 - q;
			do {
				*q = *(q + ds);
				q++;
			} while (--cp);
		} while (--lz);
	} while (q < q1);
	return 0;
err:
	return 1;
}

static int tek_decode1(int siz, UCHAR *p, UCHAR *q)
{
	int dsiz, hed, bsiz;
	UCHAR *p1 = p + siz;
	p += 16;
	if ((dsiz = tek_getnum_s7s(&p)) > 0) {
		hed = tek_getnum_s7s(&p);
		bsiz = 1 << (((hed >> 1) & 0x0f) + 8);
		if (dsiz > bsiz || (hed & 0x21) != 0x01)
			return 1;
		if (hed & 0x40)
			tek_getnum_s7s(&p); 
		if (tek_getnum_s7s(&p) != 0)
			return 1; 
		return tek_lzrestore_stk1(p1 - p, p, dsiz, q);
	}
	return 0;
}

static unsigned int tek_getnum_s7(UCHAR **pp)
{
	unsigned int s = 0, b = 0, a = 1;
	UCHAR *p = *pp;
	for (;;) {
		s = s << 7 | *p++;
		if (s & 1)
			break;
		a <<= 7;
		b += a;
	}
	s >>= 1;
	*pp = p;
	return s + b;
}

static int tek_lzrestore_stk2(int srcsiz, UCHAR *src, int outsiz, UCHAR *q)
{
	int cp, ds, repdis[4], i, j;
	UCHAR *q1 = q + outsiz, *s7ptr = src, *q0 = q, bylz, cbylz;
	for (j = 0; j < 4; j++)
		repdis[j] = -1 - j;
	bylz = cbylz = 0;
	if (outsiz) {
		if (tek_getnum_s7s(&s7ptr))
			return 1;
		do {
			j = 0;
			do {
				j++;
				if (j >= 17) {
					j += tek_getnum_s7s(&s7ptr);
					break;
				}
				if (cbylz == 0) {
					cbylz = 8;
					bylz = *s7ptr++;
				}
				cbylz--;
				i = bylz & 1;
				bylz >>= 1;
			} while (i == 0);
			do {
				*q++ = *s7ptr++;
			} while (--j);
			if (q >= q1)
				break;

			j = 0;
			do {
				j++;
				if (j >= 17) {
					j += tek_getnum_s7s(&s7ptr);
					break;
				}
				if (cbylz == 0) {
					cbylz = 8;
					bylz = *s7ptr++;
				}
				cbylz--;
				i = bylz & 1;
				bylz >>= 1;
			} while (i == 0);
			do {
				i = *s7ptr++;
				cp = i >> 4;
				i &= 0x0f;
				if ((i & 1) == 0)
					i |= (tek_getnum_s7(&s7ptr) + 1) << 4;
				i >>= 1;
				ds = ~(i - 6);
				if (i < 4)
					ds = repdis[i];
				if (i == 4)
					ds = repdis[0] - tek_getnum_s7(&s7ptr) - 1;
				if (i == 5)
					ds = repdis[0] + tek_getnum_s7(&s7ptr) + 1;
				if (cp == 0)
					cp = tek_getnum_s7(&s7ptr) + 16;
				cp++;
				if (i > 0) {
					if (i > 1) {
						if (i > 2)
							repdis[3] = repdis[2];
						repdis[2] = repdis[1];
					}
					repdis[1] = repdis[0];
					repdis[0] = ds;
				}
				if (q + ds < q0)
					goto err;
				if (q + cp > q1)
					cp = q1 - q;
				do {
					*q = *(q + ds);
					q++;
				} while (--cp);
			} while (--j);
		} while (q < q1);
	}
	return 0;
err:
	return 1;
}

static int tek_decode2(int siz, UCHAR *p, UCHAR *q)
{
	UCHAR *p1 = p + siz;
	int dsiz, hed, bsiz, st = 0;
	p += 16;
	if ((dsiz = tek_getnum_s7s(&p)) > 0) {
		hed = tek_getnum_s7s(&p);
		bsiz = 1 << (((hed >> 1) & 0x0f) + 8);
		if (dsiz > bsiz || (hed & 0x21) != 0x01)
			return 1;
		if (hed & 0x40)
			tek_getnum_s7s(&p); 
		st = tek_lzrestore_stk2(p1 - p, p, dsiz, q);
	}
	return st;
}

static int tek_decmain5(int *work, UCHAR *src, int osiz, UCHAR *q, int lc, int pb, int lp, int flags);

static int tek_lzrestore_tek5(int srcsiz, UCHAR *src, int outsiz, UCHAR *outbuf)
{
	int wrksiz, lc, lp, pb, flags, *work, prop0, fl;

	if ((fl = (prop0 = *src) & 0x0f) == 0x01) /* 0001 */
		flags |= -1;
	else if (fl == 0x05)
		flags = -2;
	else if (fl == 0x09)
		flags &= 0;
	else
		return 1;
	src++;
	prop0 >>= 4;
	if (prop0 == 0)
		prop0 = *src++;
	else {
		static UCHAR prop0_table[] = { 0x5d, 0x00 }, prop1_table[] = { 0x00 };
		if (flags == -1) {
			if (prop0 >= 3)
				return 1;
			prop0 = prop0_table[prop0 - 1];
		} else {
			if (prop0 >= 2)
				return 1;
			prop0 = prop1_table[prop0 - 1];
		}
	}
	lp = prop0 / (9 * 5);
	prop0 %= 9 * 5;
	pb = prop0 / 9;
	lc = prop0 % 9;
	if (flags == 0) /* tek5:z2 */
		flags = *src++;
	if (flags == -1) { /* stk5 */
		wrksiz = lp;
		lp = pb;
		pb = wrksiz;
	}
	wrksiz = 0x180 * sizeof (UINT32) + (0x840 + (0x300 << (lc + lp))) * sizeof (tek_TPRB); /* Å’á15KB, lc+lp=3‚È‚çA36KB */
	work = (int *) memman_alloc_4k((struct MEMMAN *) MEMMAN_ADDR, wrksiz);
	if (work == NULL)
		return -1;
	flags = tek_decmain5(work, src, outsiz, outbuf, lc, pb, lp, flags);
	memman_free_4k((struct MEMMAN *) MEMMAN_ADDR, (int) work, wrksiz);
	return flags;
}

struct tek_STR_BITMODEL {
	UCHAR t, m, s, dmy;
	UINT32 prb0, prb1, tmsk, ntm, lt, lt0, dmy4;
};

struct tek_STR_PRB {
	struct tek_STR_PRB_PB {
		struct tek_STR_PRB_PBST {
			tek_TPRB mch, rep0l1;
		} st[12];
		tek_TPRB lenlow[2][8], lenmid[2][8];
	} pb[16];
	struct tek_STR_PRB_ST {
		tek_TPRB rep, repg0, repg1, repg2;
	} st[12];
	tek_TPRB lensel[2][2], lenhigh[2][256], pslot[4][64], algn[64];
	tek_TPRB spdis[2][2+4+8+16+32], lenext[2+4+8+16+32];
	tek_TPRB repg3, fchgprm[2 * 32], tbmt[16], tbmm[16], fchglt;
	tek_TPRB lit[1];
};

struct tek_STR_RNGDEC {
	UCHAR *p;
	UINT32 range, code, rmsk;
	jmp_buf errjmp;
	struct tek_STR_BITMODEL bm[32], *ptbm[16];
	struct tek_STR_PRB probs;
};

static void tek_setbm5(struct tek_STR_BITMODEL *bm, int t, int m)
{
	bm->t = t;
	bm->m = m;
	bm->prb1 = -1 << (m + t);
	bm->prb0 = ~bm->prb1;
	bm->prb1 |= 1 << t;
	bm->tmsk = (-1 << t) & 0xffff;
	bm->prb0 &= bm->tmsk;
	bm->prb1 &= bm->tmsk;
	bm->ntm = ~bm->tmsk;
	return;
}

static int tek_rdget0(struct tek_STR_RNGDEC *rd, int n, int i)
{
	do {
		while (rd->range < (UINT32) (1 << 24)) {
			rd->range <<= 8;
			rd->code = rd->code << 8 | *rd->p++;
		}
		rd->range >>= 1;
		i += i;
		if (rd->code >= rd->range) {
			rd->code -= rd->range;
			i |= 1;
		}
	} while (--n);
	return ~i;
}

static int tek_rdget1(struct tek_STR_RNGDEC *rd, tek_TPRB *prob0, int n, int j, struct tek_STR_BITMODEL *bm)
{
	UINT32 p, i, *prob, nm = n >> 4;
	n &= 0x0f;
	prob0 -= j;
	do {
		p = *(prob = prob0 + j);
		if (bm->lt > 0) {
			if (--bm->lt == 0) {
				if (tek_rdget1(rd, &rd->probs.fchglt, 0x71, 0, &rd->bm[3]) == 0) {
err:
					longjmp(rd->errjmp, 1);
				}
				i = bm - rd->bm;
				if ((bm->s = tek_rdget1(rd, &rd->probs.fchgprm[i * 2 + bm->s], 0x71, 0, &rd->bm[1])) == 0) {
					i = tek_rdget1(rd, rd->probs.tbmt, 0x74, 1, &rd->bm[2]) & 15;
					if (i == 15)
						goto err;
					tek_setbm5(bm, i, ((tek_rdget1(rd, rd->probs.tbmm, 0x74, 1, &rd->bm[2]) - 1) & 15) + 1);
				}
				bm->lt = bm->lt0;
			}
			if (p < bm->prb0) {
				p = bm->prb0;
				goto fixprob;
			}
			if (p > bm->prb1) {
				p = bm->prb1;
				goto fixprob;
			}
			if (p & bm->ntm) {
				p &= bm->tmsk;
	fixprob:
				*prob = p;
			}
		}

		while (rd->range < (UINT32) (1 << 24)) {
			rd->range <<= 8;
			rd->code = rd->code << 8 | *rd->p++;
		}
		j += j;
		i = ((unsigned long long) (rd->range & rd->rmsk) * p) >> 16;
		if (rd->code < i) {
			j |= 1;
			rd->range = i;
			*prob += ((0x10000 - p) >> bm->m) & bm->tmsk;
		} else {
			rd->range -= i;
			rd->code -= i;
			*prob -= (p >> bm->m) & bm->tmsk;
		}
		--n;
		if ((n & nm) == 0)
			bm++;
	} while (n);
	return j;
}

static UINT32 tek_revbit(UINT32 data, int len)
{
	UINT32 rev = 0;
	do {
		rev += rev + (data & 1);
		data >>= 1;
	} while (--len);
	return rev;
}

static int tek_getlen5(struct tek_STR_RNGDEC *rd, int m, int s_pos, int stk)
{
	int i;
	if (tek_rdget1(rd, &rd->probs.lensel[m][0], 0x71, 0, rd->ptbm[3]) ^ stk) /* low */
		i = tek_rdget1(rd, rd->probs.pb[s_pos].lenlow[m], 0x73, 1, rd->ptbm[4]) & 7;
	else if (tek_rdget1(rd, &rd->probs.lensel[m][1], 0x71, 0, rd->ptbm[3]) ^ stk) /* mid */
		i = tek_rdget1(rd, rd->probs.pb[s_pos].lenmid[m], 0x73, 1, rd->ptbm[5]);
	else {
		/* high */
		i = tek_rdget1(rd, rd->probs.lenhigh[m], 0x78, 1, rd->ptbm[6]) - (256 + 256 - 8);
		if (i > 0) {
			if (i < 6 && stk == 0)
				i = tek_rdget1(rd, &rd->probs.lenext[(1 << i) - 2], i | 0x70, 1, rd->ptbm[7]) - 1;
			else
				i = tek_rdget0(rd, i, ~1) - 1;
			i = tek_rdget0(rd, i, ~1) - 1;
		}
		i += 256 - 8 + 16;
	}
	return i;
}

static int tek_decmain5(int *work, UCHAR *src, int osiz, UCHAR *q, int lc, int pb, int lp, int flags)
{
	static int state_table[] = { 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 4, 5 };
	int i, j, k, pmch, rep[4], s, pos, m_pos = (1 << pb) - 1, m_lp = (1 << lp) - 1;
	int stk = (flags == -1), lcr = 8 - lc, s_pos, lit0cntmsk = 0x78;
	UINT32 *lit1;
	struct tek_STR_RNGDEC *rd = (struct tek_STR_RNGDEC *) work;
	struct tek_STR_PRB *prb = &rd->probs;

	rd->p = &src[4];
	rd->range |= -1;
	rd->code = src[0] << 24 | src[1] << 16 | src[2] << 8 | src[3];
	for (i = 0; i < 4; i++)
		rep[i] = ~i;
	if (setjmp(rd->errjmp))
		goto err;
	for (i = sizeof (struct tek_STR_PRB) / sizeof (tek_TPRB) + (0x300 << (lc + lp)) - 2; i >= 0; i--)
		((tek_TPRB *) prb)[i] = 1 << 15;
	for (i = 0; i < 32; i++) {
		rd->bm[i].lt = (i >= 4);
		rd->bm[i].lt0 = (i < 24) ? 16 * 1024 : 8 * 1024;
		rd->bm[i].s &= 0;
		rd->bm[i].t = rd->bm[i].m = 5;
	}
	lit1 = prb->lit + ((256 << (lc + lp)) - 2);
	if (stk) {
		rd->rmsk = -1 << 11;
		for (i = 0; i < 32; i++)
			rd->bm[i].lt = 0;
		for (i = 0; i < 14; i++)
			rd->ptbm[i] = &rd->bm[0];
	} else {
		UCHAR pt[14];
		static UCHAR pt1[14] = {
			 8,  8,  8,  8,  8,  8,  8,  8,
			 8,  8, 18, 18, 18,  8
		};
		static UCHAR pt2[14] = {
			 8,  8, 10, 11, 12, 12, 14, 15,
			16, 16, 18, 18, 20, 21
		};
		/*
			 0- 7:mch, mch, lit1, lensel, lenlow, lenmid, lenhigh, lenext
			 8-15:pslot, pslot, sdis, sdis, align, rep-repg2
		*/
		rd->rmsk |= -1;
		rd->bm[1].t = 5; rd->bm[1].m = 3; /* for fchgprm */
		rd->bm[2].t = 9; rd->bm[2].m = 2; /* for tbmt, tbmm */
		if (flags & 0x40) { /* lt-flag */
			rd->bm[3].t = 0; rd->bm[3].m = 1;
			prb->fchglt = 0xffff;
		}
		rd->bm[22].t = 0; rd->bm[22].m = 1;
		prb->repg3 = 0xffff;
		if (flags == -2) { /* z1 */
			rd->bm[22].lt = 0;
			for (i = 0; i < 14; i++)
				pt[i] = pt1[i];
		} else {
			for (i = 0; i < 14; i++)
				pt[i] = pt2[i];
			lit0cntmsk = (7 >> (flags & 3)) << 4 | 8;
			pt[ 1] =  8 + ((flags & 0x04) != 0); /* mch */
			pt[ 5] = 12 + ((flags & 0x08) != 0); /* llm */
			pt[ 9] = 16 + ((flags & 0x10) != 0); /* pst */
			pt[11] = 18 + ((flags & 0x20) != 0); /* sds */
		}
		for (i = 0; i < 14; i++)
			rd->ptbm[i] = &rd->bm[pt[i]];
	}
	for (i = 0; i < 32; i++)
		tek_setbm5(&rd->bm[i], rd->bm[i].t, rd->bm[i].m);

	if ((tek_rdget1(rd, &prb->pb[0].st[0].mch, 0x71, 0, rd->ptbm[0]) ^ stk) == 0)
		goto err;
	*q++ = tek_rdget1(rd, prb->lit, lit0cntmsk, 1, &rd->bm[24]) & 0xff;
	pmch &= 0; s &= 0; pos = 1;
	while (pos < osiz) {
		s_pos = pos & m_pos;
		if (tek_rdget1(rd, &prb->pb[s_pos].st[s].mch, 0x71, 0, rd->ptbm[s > 0]) ^ stk) {
			i = (q[-1] >> lcr | (pos & m_lp) << lc) << 8;
			s = state_table[s];
			if (pmch == 0)
				*q = tek_rdget1(rd, &prb->lit[i], lit0cntmsk, 1, &rd->bm[24]) & 0xff;
			else {
				struct tek_STR_BITMODEL *bm = &rd->bm[24];
				j = 1; 
				k = 8;
				pmch = q[rep[0]];
				do {
					j += j + tek_rdget1(rd, &lit1[(i + j) << 1 | pmch >> 7], 0x71, 0, rd->ptbm[2]);
					k--;
					if ((k & (lit0cntmsk >> 4)) == 0)
						bm++;
					if ((((pmch >> 7) ^ j) & 1) != 0 && k != 0) {
						j = tek_rdget1(rd, &prb->lit[i + j - 1], k | (lit0cntmsk & 0x70), j, bm);
						break;
					}
					pmch <<= 1;
				} while (k);
				*q = j & 0xff;
				pmch &= 0;
			}
			pos++;
			q++;
		} else { /* lz */
			pmch |= 1;
			if (tek_rdget1(rd, &prb->st[s].rep, 0x71, 0, rd->ptbm[13]) ^ stk) { /* len/dis */
				rep[3] = rep[2];
				rep[2] = rep[1];
				rep[1] = rep[0];
				j = i = tek_getlen5(rd, 0, s_pos, stk);
				s = s < 7 ? 7 : 10;
				if (j >= 4)
					j = 3;
				rep[0] = j = tek_rdget1(rd, prb->pslot[j], 0x76, 1, rd->ptbm[8 + (j == 3)]) & 0x3f;
				if (j >= 4) {
					k = (j >> 1) - 1; /* k = [1, 30] */
					rep[0] = (2 | (j & 1)) << k;
					if (j < 14) /* k < 6 */
						rep[0] |= tek_revbit(tek_rdget1(rd, &prb->spdis[j & 1][(1 << k) - 2], k | 0x70, 1, rd->ptbm[10 + (k >= 4)]), k);
					else {
						if (stk == 0) {
							if (k -= 6)
								rep[0] |= tek_rdget0(rd, k, ~0) << 6;
							rep[0] |= tek_revbit(tek_rdget1(rd, prb->algn, 0x76, 1, rd->ptbm[12]), 6);
						} else {
							rep[0] |= tek_rdget0(rd, k - 4, ~0) << 4;
							rep[0] |= tek_revbit(tek_rdget1(rd, prb->algn, 0x74, 1, rd->ptbm[12]), 4);
						}
					}
				}
				rep[0] = ~rep[0];
			} else { /* repeat-dis */
				if (tek_rdget1(rd, &prb->st[s].repg0, 0x71, 0, rd->ptbm[13]) ^ stk) { /* rep0 */
					i |= -1;
					if (tek_rdget1(rd, &prb->pb[s_pos].st[s].rep0l1, 0x71, 0, rd->ptbm[13]) == 0) {
						s = s < 7 ? 9 : 11;
						goto skip;
					}
				} else {
					if (tek_rdget1(rd, &prb->st[s].repg1, 0x71, 0, rd->ptbm[13]) ^ stk) /* rep1 */
						i = rep[1];
					else {
						if (tek_rdget1(rd, &prb->st[s].repg2, 0x71, 0, rd->ptbm[13]) ^ stk) /* rep2 */
							i = rep[2];
						else {
							if (stk == 0) {
								if  (tek_rdget1(rd, &prb->repg3, 0x71, 0, &rd->bm[22]) == 0)
									goto err;
							}
							i = rep[3]; /* rep3 */
							rep[3] = rep[2];
						}
						rep[2] = rep[1];
					}
					rep[1] = rep[0];
					rep[0] = i;
				}
				i = tek_getlen5(rd, 1, s_pos, stk);
				s = s < 7 ? 8 : 11;
			}
skip:
			i += 2;
			if (pos + rep[0] < 0)
				goto err;
			if (pos + i > osiz)
				i = osiz - pos;
			pos += i;
			do {
				*q = q[rep[0]];
				q++;
			} while (--i);
		}
	}
	return 0;
err:
	return 1;
}

static int tek_decode5(int siz, UCHAR *p, UCHAR *q)
{
	UCHAR *p1 = p + siz;
	int dsiz, hed, bsiz, st = 0;
	p += 16;
	if ((dsiz = tek_getnum_s7s(&p)) > 0) {
		hed = tek_getnum_s7s(&p);
		if ((hed & 1) == 0)
			st = tek_lzrestore_tek5(p1 - p + 1, p - 1, dsiz, q);
		else {
			bsiz = 1 << (((hed >> 1) & 0x0f) + 8);
			if (hed & 0x20)
				return 1;
			if (bsiz == 256)
				st = tek_lzrestore_tek5(p1 - p, p, dsiz, q);
			else {
				if (dsiz > bsiz)
					return 1;
				if (hed & 0x40)
					tek_getnum_s7s(&p);
				st = tek_lzrestore_tek5(p1 - p, p, dsiz, q);
			}
		}
	}
	return st;
}
