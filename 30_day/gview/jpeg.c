/*
 * JPEG decoding engine for DCT-baseline
 *
 *      copyrights 2003 by nikq | nikq::club.
 *
 *
 */


typedef unsigned char UCHAR;

struct DLL_STRPICENV { int work[16384]; };

typedef struct
{
    int elem; 
    unsigned short code[256];
    unsigned char  size[256];
    unsigned char  value[256];
}HUFF;

typedef struct
{
    // SOF
    int width;
    int height;
    // MCU
    int mcu_width;
    int mcu_height;

    int max_h,max_v;
    int compo_count;
    int compo_id[3];
    int compo_sample[3];
    int compo_h[3];
    int compo_v[3];
    int compo_qt[3];

    // SOS
    int scan_count;
    int scan_id[3];
    int scan_ac[3];
    int scan_dc[3];
    int scan_h[3];  
    int scan_v[3];  
    int scan_qt[3]; 
    
    // DRI
    int interval;

    int mcu_buf[32*32*4]; 
    int *mcu_yuv[4];
    int mcu_preDC[3];
    
    // DQT
    int dqt[3][64];
    int n_dqt;
    
    // DHT
    HUFF huff[2][3];
    
    
    // FILE i/o
	unsigned char *fp, *fp1;
    unsigned long bit_buff;
    int bit_remain;
    int width_buf;

	int base_img[64][64]; 

    /* for dll 
    
    JPEG *jpeg = (JPEG *)malloc(sizeof(JPEG) + 256);
    */
    int dummy[64];
    
}JPEG;

/* for 16bit */
#ifndef PIXEL16
#define PIXEL16(r, g, b)	((r) << 11 | (g) << 5 | (b))
	/* 0 <= r <= 31, 0 <= g <= 63, 0 <= b <= 31 */
#endif

int info_JPEG(struct DLL_STRPICENV *env, int *info, int size, UCHAR *fp);
int decode0_JPEG(struct DLL_STRPICENV *env, int size, UCHAR *fp, int b_type, UCHAR *buf, int skip);

void jpeg_idct_init(int base_img[64][64]);
int jpeg_init(JPEG *jpeg);
// int jpeg_header(JPEG *jpge);
void jpeg_decode(JPEG *jpeg, unsigned char *rgb,int b_type);

/* ----------------- start main section ----------------- */

int info_JPEG(struct DLL_STRPICENV *env,int *info, int size, UCHAR *fp0)
{
	JPEG *jpeg = (JPEG *) (((int *) env) + 128);
	jpeg->fp = fp0;
	jpeg->fp1 = fp0 + size;

//	if (512 + sizeof (JPEG) > 64 * 1024)
//		return 0;

	if (jpeg_init(jpeg))
		return 0;
//	jpeg_header(jpeg);

	if (jpeg->width == 0)
		return 0;
	
	info[0] = 0x0002;
	info[1] = 0x0000;
	info[2] = jpeg->width;
	info[3] = jpeg->height;

	/* OK */
	return 1;
}

int decode0_JPEG(struct DLL_STRPICENV *env,int size, UCHAR *fp0, int b_type, UCHAR *buf, int skip)
{
	JPEG *jpeg = (JPEG *) (((int *) env) + 128);
	jpeg->fp = fp0;
	jpeg->fp1 = fp0 + size;

	jpeg_idct_init(jpeg->base_img);
	jpeg_init(jpeg);
//	jpeg_header(jpeg);

//	if (jpeg->width == 0)
//		return 8;
	/* decode*/

	jpeg->width_buf = skip / (b_type & 0x7f) + jpeg->width;
    jpeg_decode(jpeg, buf, b_type);

	/* OK */
	return 0;
}

// -------------------------- I/O ----------------------------

unsigned short get_bits(JPEG *jpeg, int bit)
{
	unsigned char  c, c2;
	unsigned short ret;
	unsigned long  buff;
	int remain;

	buff   = jpeg->bit_buff;
	remain = jpeg->bit_remain;

	while (remain <= 16) {
		if (jpeg->fp >= jpeg->fp1) {
			ret = 0;
			goto fin;
		}
		c = *jpeg->fp++;
		if (c == 0xff) { 
			if (jpeg->fp >= jpeg->fp1) {
				ret = 0;
				goto fin;
			}
			jpeg->fp++; /* 00 skip */
		}
		buff = (buff << 8) | c;
		remain += 8;
	}
	ret = (buff >> (remain - bit)) & ((1 << bit) - 1);
	remain -= bit;

	jpeg->bit_remain = remain;
	jpeg->bit_buff   = buff;
fin:
	return ret;
}

// ------------------------ JPEG -----------------

// start of frame
int jpeg_sof(JPEG *jpeg)
{
	unsigned char c;
	int i, h, v, n;

	if (jpeg->fp + 8 > jpeg->fp1)
		goto err;
	/* fp[2] ‚Í bpp */
	jpeg->height = jpeg->fp[3] << 8 | jpeg->fp[4];
	jpeg->width  = jpeg->fp[5] << 8 | jpeg->fp[6];
	n = jpeg->compo_count = jpeg->fp[7]; // Num of compo, nf
	jpeg->fp += 8;
	if (jpeg->fp + n * 3 > jpeg->fp1)
		goto err;

	for (i = 0; i < n; i++) {
		jpeg->compo_id[i] = jpeg->fp[0];

		jpeg->compo_sample[i] = c = jpeg->fp[1];
		h = jpeg->compo_h[i] = (c >> 4) & 0x0f;
		v = jpeg->compo_v[i] = c & 0x0f;

		if (jpeg->max_h < h)
			jpeg->max_h = h;
		if (jpeg->max_v < v)
			jpeg->max_v = v;

        jpeg->compo_qt[i] = jpeg->fp[2];
		jpeg->fp += 3;
    }
    return 0;
err:
	jpeg->fp = jpeg->fp1;
	return 1;
}

// define quantize table
int jpeg_dqt(JPEG *jpeg)
{
	unsigned char c;
	int i, j, v, size;

	if (jpeg->fp + 2 > jpeg->fp1)
		goto err;
	size = (jpeg->fp[0] << 8 | jpeg->fp[1]) - 2;
	jpeg->fp += 2;
	if (jpeg->fp + size > jpeg->fp1)
		goto err;

	while (size > 0) {
		c = *jpeg->fp++;
		size--;
		j = c & 7;
		if (j > jpeg->n_dqt)
			jpeg->n_dqt = j;

 		if (c & 0xf8) {
			// 16 bit DQT
			for (i = 0; i < 64; i++) {
				jpeg->dqt[j][i] = jpeg->fp[0]; 
				jpeg->fp += 2;
			}
			size += -64 * 2;
		} else {
			//  8 bit DQT
			for (i = 0; i < 64; i++)
				jpeg->dqt[j][i] = *jpeg->fp++;
			size -= 64;
		}
	}
	return 0;
err:
	jpeg->fp = jpeg->fp1;
	return 1;
}

// define huffman table
int jpeg_dht(JPEG *jpeg)
{
	unsigned tc, th;
	unsigned code = 0;
	unsigned char val;
	int i, j, k, num, Li[17];
	int len, max_val;
	HUFF *table;

	if (jpeg->fp + 2 > jpeg->fp1)
		goto err;
	len = (jpeg->fp[0] << 8 | jpeg->fp[1]) - 2;
	jpeg->fp += 2;

	while (len > 0) {
		if (jpeg->fp + 17 > jpeg->fp1)
			goto err;
		val = jpeg->fp[0];

		tc = (val >> 4) & 0x0f; 
		th =  val       & 0x0f; 
		table = &(jpeg->huff[tc][th]);

		num = 0;
		k = 0;
		for (i = 1; i <= 16; i++) {
			Li[i] = jpeg->fp[i];
			num += Li[i];
            for (j = 0; j < Li[i]; j++)
                table->size[k++] = i;
		}
		table->elem = num;
		jpeg->fp += 17;

		k=0;
		code=0;
		i = table->size[0];
		while (k < num) {
			while (table->size[k] == i)
				table->code[k++] = code++;
			if (k >= num)
				break;
			do {
				code <<= 1;
				i++;
			} while (table->size[k] != i);
		}

		if (jpeg->fp + num > jpeg->fp1)
			goto err;
		for (k = 0; k < num; k++)
			table->value[k] = jpeg->fp[k];
		jpeg->fp += num;

        len -= 18 + num;
    }
    return 0;
err:
	jpeg->fp = jpeg->fp1;
	return 1;
}

int jpeg_init(JPEG *jpeg)
{
	unsigned char c;
	int r = 0, i;
	jpeg->width = 0;
	jpeg->mcu_preDC[0] = 0;
	jpeg->mcu_preDC[1] = 0;
	jpeg->mcu_preDC[2] = 0;
	jpeg->n_dqt = 0;
	jpeg->max_h = 0;
	jpeg->max_v = 0;
	jpeg->bit_remain = 0;
	jpeg->bit_buff   = 0;

	jpeg->interval = 0;
//	return;
//}
//
//int jpeg_header(JPEG *jpeg)
//{
//	unsigned char c;
//	int r = 0, i;

	for (;;) {
		if (jpeg->fp + 2 > jpeg->fp1)
			goto err;
		if (jpeg->fp[0] != 0xff)
            goto err0;
		c = jpeg->fp[1];
		jpeg->fp += 2;
		if (c == 0xd8)
			continue; /* SOI */
		if (c == 0xd9)
			goto err; /* EOI */

		if (c == 0xc0)
            jpeg_sof(jpeg);
		else if (c == 0xc4)
            jpeg_dht(jpeg);
		else if (c == 0xdb)
            jpeg_dqt(jpeg);
		else if (c == 0xdd) {
			if (jpeg->fp + 4 > jpeg->fp1)
				goto err;
			jpeg->interval = jpeg->fp[2] << 8 | jpeg->fp[3];
			jpeg->fp += 4;
		} else if (c == 0xda) {
			if (jpeg->fp + 3 > jpeg->fp1)
				goto err;
			jpeg->scan_count = jpeg->fp[2];
			jpeg->fp += 3;
			if (jpeg->fp + jpeg->scan_count * 2 > jpeg->fp1)
				goto err;
			for (i = 0; i < jpeg->scan_count; i++) {
				jpeg->scan_id[i] = jpeg->fp[0];
				jpeg->scan_dc[i] = jpeg->fp[1] >> 4;   // DC Huffman Table
				jpeg->scan_ac[i] = jpeg->fp[1] & 0x0F; // AC Huffman Table
				jpeg->fp += 2;
			}
			jpeg->fp += 3; /* 3bytes skip */
            goto fin;
		} else {
			if (jpeg->fp + 2 > jpeg->fp1)
				goto err;
			jpeg->fp += jpeg->fp[0] << 8 | jpeg->fp[1];
		}
    }
err:
	jpeg->fp = jpeg->fp1;
err0:
	r++;
fin:
	return r;
}

// MCU decode

void jpeg_decode_init(JPEG *jpeg)
{
	int i, j;

	for (i = 0; i < jpeg->scan_count; i++) {
		// i:scan
		for (j = 0; j < jpeg->compo_count; j++) {
			// j:frame
			if (jpeg->scan_id[i] == jpeg->compo_id[j]) {
				jpeg->scan_h[i]  = jpeg->compo_h[j];
				jpeg->scan_v[i]  = jpeg->compo_v[j];
				jpeg->scan_qt[i] = jpeg->compo_qt[j];
                break;
			}
		}
	//	if (j >= jpeg->compo_count)
	//		return 1;
	}
	jpeg->mcu_width  = jpeg->max_h * 8;
	jpeg->mcu_height = jpeg->max_v * 8;
    
	for (i = 0; i < 32 * 32 * 4; i++)
		jpeg->mcu_buf[i] = 0x80;

	for (i = 0; i < jpeg->scan_count; i++)
		jpeg->mcu_yuv[i] = jpeg->mcu_buf + (i << 10);
	return;
}

int jpeg_huff_decode(JPEG *jpeg,int tc,int th)
{
    HUFF *h = &(jpeg->huff[tc][th]);
    int code,size,k,v;
    code = 0;
    size = 0;
    k = 0;
    while( size < 16 ){
        size++;
        v = get_bits(jpeg,1);
        if(v < 0){
            return v;
        }
        code = (code << 1) | v;

        while(h->size[k] == size){
            if(h->code[k] == code){
                return h->value[k];
            }
            k++;
        }
    }

    return -1;
}

void jpeg_idct_init(int base_img[64][64])
{
    int u, v, m, n;
    int tmpm[8], tmpn[8];
	int cost[32];
	cost[ 0] =  32768; cost[ 1] =  32138; cost[ 2] =  30274; cost[ 3] =  27246; cost[ 4] =  23170; cost[ 5] =  18205; cost[ 6] =  12540; cost[ 7] =   6393;
	cost[ 8] =      0; cost[ 9] =  -6393; cost[10] = -12540; cost[11] = -18205; cost[12] = -23170; cost[13] = -27246; cost[14] = -30274; cost[15] = -32138;
	for (u = 0; u < 16; u++)
		cost[u + 16] = - cost[u];

    for (u = 0; u < 8; u++) {
        {
            int i=u, d=u*2;
            if (d == 0)
                i = 4;
            for (m = 0; m < 8; m++){
                tmpm[m] = cost[i];
                i=(i+d)&31;
            }
        }
        for (v = 0; v < 8; v++) {
            {
                int i=v,d=v*2;
                if (d == 0)
                    i=4;
                for (n = 0; n < 8; n++){
                    tmpn[n] = cost[i]; 
                    i=(i+d)&31;
                }
            }
            
            for (m = 0; m < 8; m++) {
                for (n = 0; n < 8; n++) {
                    base_img[u * 8 + v][m * 8 + n] = (tmpm[m] * tmpn[n])>>15;
                }
            }
        }
    }
    return;
}

void jpeg_idct(int *block, int *dest, int base_img[64][64])
{
    int i, j ,k;

    for (i = 0; i < 64; i++)
        dest[i] = 0;

    for (i = 0; i < 64; i++) {
        k = block[i];
        if(k) { 
            for (j = 0; j < 64; j++) {
                dest[j] += k * base_img[i][j];
            }
        }
    }
    
    for (i = 0; i < 64; i++)
        dest[i] >>= 17;
    return;
}


int jpeg_get_value(JPEG *jpeg,int size)
{
	int val = 0;
    if (size) {
		val = get_bits(jpeg,size);
		if (val < 0)
			val = 0x10000 | (0 - val);
		else if (!(val & (1<<(size-1))))
			val -= (1 << size) - 1;
	}
    return val;
}

int jpeg_decode_huff(JPEG *jpeg,int scan,int *block, UCHAR *zigzag_table)
{
    int size, len, val, run, index;
    int *pQt = (int *)(jpeg->dqt[jpeg->scan_qt[scan]]);

    // DC
    size = jpeg_huff_decode(jpeg,0,jpeg->scan_dc[scan]);
    if(size < 0)
        return 0;
    val = jpeg_get_value(jpeg,size);
    jpeg->mcu_preDC[scan] += val;
    block[0] = jpeg->mcu_preDC[scan] * pQt[0];

    //AC
    index = 1;
    while(index<64)
    {
        size = jpeg_huff_decode(jpeg,1,jpeg->scan_ac[scan]);
        if(size < 0)
            break;
        // EOB
        if(size == 0)
            break;
        
        // RLE
        run  = (size>>4)&0xF;
        size = size & 0x0F;
        
        val = jpeg_get_value(jpeg,size);
        if(val>=0x10000) {
            return val;
        }

        // ZRL
        while (run-- > 0)
            block[ zigzag_table[index++] ] = 0;
        
        block[ zigzag_table[index] ] = val * pQt[index];
        index++;
    }
    while(index<64)
        block[zigzag_table[index++]]=0;
    return 0;
}

void jpeg_mcu_bitblt(int *src, int *dest, int width,
                     int x0, int y0, int x1, int y1)
{
	int w, h;
	int x, y, x2, y2;
	w = x1 - x0;
	h = y1 - y0;
	dest += y0 * width + x0;
	width -= w;

	for (y = 0; y < h; y++) {
		y2 = (y * 8 / h) * 8;
		for (x = 0; x < w; x++)
			*dest++ = src[y2 + (x * 8 / w)];
		dest += width;
	}
}

int jpeg_decode_mcu(JPEG *jpeg, UCHAR *zigzag_table)
{
	int scan, val;
	int unit, i, h, v;
	int *p, hh, vv;
	int block[64], dest[64];

	// mcu_width x mcu_height
	for (scan = 0; scan < jpeg->scan_count; scan++) {
		hh = jpeg->scan_h[scan];
		vv = jpeg->scan_v[scan];
		for (v = 0; v < vv; v++) {
            for (h = 0; h < hh; h++) {
				// ƒuƒƒbƒN(8x8)‚ÌƒfƒR[ƒh
				val = jpeg_decode_huff(jpeg, scan, block, zigzag_table);
			//	if(val>=0x10000){
			//		printf("marker found:%02x\n",val);
			//	}

				jpeg_idct(block, dest, jpeg->base_img);

				p = jpeg->mcu_buf + (scan << 10);

				jpeg_mcu_bitblt(dest, p, jpeg->mcu_width,
					jpeg->mcu_width * h / hh, jpeg->mcu_height * v / vv,
					jpeg->mcu_width * (h + 1) / hh, jpeg->mcu_height * (v + 1) / vv);
			}
		}
	}
}

// YCrCb=>RGB

int jpeg_decode_yuv(JPEG *jpeg, int h, int v, unsigned char *rgb, int b_type)
{
	int x0, y0, x, y, x1, y1;
	int *py;
	int Y12, V;
	int mw, mh, w;
	int i;

	mw = jpeg->mcu_width;
	mh = jpeg->mcu_height;

	x0 = h * jpeg->max_h * 8;
	y0 = v * jpeg->max_v * 8;

	x1 = jpeg->width - x0;
	if (x1 > mw)
		x1 = mw;
	y1 = jpeg->height - y0;
	if (y1 > mh)
		y1 = mh;
    
	py = jpeg->mcu_buf;
	rgb += (y0 * jpeg->width_buf + x0) * (b_type & 0x7f);
	w = (jpeg->width_buf - x1) * (b_type & 0x7f);

	for (y = 0; y < y1; y++) {
		for (x = 0; x < x1; x++) {
			int b, g, r;
			Y12 = py[0] << 12;
		//	U = py[1024]; /* pu */
			V = py[2048]; /* pv */

			/* blue */
			b = 128 + ((Y12 - V * 4 + py[1024] /* pu */ * 0x1C59) >> 12);
			if (b & 0xffffff00)
				b = (~b) >> 24;

			/* green */
			g = 128 + ((Y12 - V * 0x0B6C) >> 12);
			if (g & 0xffffff00)
				g = (~g) >> 24;

			/* red */
			r = 128 + ((Y12 + V * 0x166E) >> 12);
			if (r & 0xffffff00)
				r = (~r) >> 24;
			if (b_type == 0x0004) {
				rgb[0] = b;
				rgb[1] = g;
				rgb[2] = r;
				py++;
				rgb += 4;
			} else if (b_type == 0x0002) {
				r &= 0xff;
				g &= 0xff;
				b &= 0xff;
				*(short *) rgb = PIXEL16(r >> 3, g >> 2, b >> 3);
				rgb += 2;
			}
		}
		py += mw - x1;
		rgb += w;
	}
	return;
}

#define INIT_ZTABLE(i, b0, b1, b2, b3)	*(int *) &zigzag_table[i] = b0 | b1 << 8 | b2 << 16 | b3 << 24

void jpeg_decode(JPEG *jpeg, UCHAR *rgb, int b_type)
{
	int h_unit, v_unit;
	int mcu_count, h, v;
	int val;
	unsigned char m;

    UCHAR zigzag_table[64];

	INIT_ZTABLE( 0,  0,  1,  8, 16); INIT_ZTABLE( 4,  9,  2,  3, 10);
	INIT_ZTABLE( 8, 17, 24, 32, 25); INIT_ZTABLE(12, 18, 11,  4,  5);
	INIT_ZTABLE(16, 12, 19, 26, 33); INIT_ZTABLE(20, 40, 48, 41, 34);
	INIT_ZTABLE(24, 27, 20, 13,  6); INIT_ZTABLE(28,  7, 14, 21, 28);
	INIT_ZTABLE(32, 35, 42, 49, 56); INIT_ZTABLE(36, 57, 50, 43, 36);
	INIT_ZTABLE(40, 29, 22, 15, 23); INIT_ZTABLE(44, 30, 37, 44, 51);
	INIT_ZTABLE(48, 58, 59, 52, 45); INIT_ZTABLE(52, 38, 31, 39, 46);
	INIT_ZTABLE(56, 53, 60, 61, 54); INIT_ZTABLE(60, 47, 55, 62, 63);

	jpeg_decode_init(jpeg);

	h_unit = (jpeg->width + jpeg->mcu_width - 1) / jpeg->mcu_width;
	v_unit = (jpeg->height + jpeg->mcu_height - 1) / jpeg->mcu_height;

	mcu_count = 0;
	for (v = 0; v < v_unit; v++) {
		for (h = 0; h < h_unit; h++) {
			mcu_count++;
			jpeg_decode_mcu(jpeg, zigzag_table);
			jpeg_decode_yuv(jpeg, h, v, rgb, b_type & 0x7fff);
			if (jpeg->interval > 0 && mcu_count >= jpeg->interval) {
				jpeg->bit_remain -= (jpeg->bit_remain & 7);
				jpeg->bit_remain -= 8;
				jpeg->mcu_preDC[0] = 0;
				jpeg->mcu_preDC[1] = 0;
				jpeg->mcu_preDC[2] = 0;
				mcu_count = 0;
			}
		}
	}
    return;
}

