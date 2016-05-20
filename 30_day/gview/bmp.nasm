; BMP decode routine by I.Tak. 2003

section .text align=1
[bits 32]
;BMP File Structure (I can't understand MS.)

	struc BMP
		;FILE HEADER
.fType:		resw 1	;BM
.fSize:		resd 1	;whole file size
		resd 1	;reserved
.fOffBits:	resd 1	;offset from file top to image
		;INFO HEADER
.iSize:		resd 1	;INFO HEADER size
.iWidth:	resd 1	;Image Width in pixels
.iHeight:	resd 1	;Image Height in pixels
.iPlanes:	resw 1	;must be 1
.iBitCount:	resw 1	;BitPerPixel 1, 4, 8, 24 (and 15,16 for new OS/2 ?)
.iCompression:	resd 1	;Compress Type. 0 for none, then SizeImage=0
.iSizeImage:	resd 1	;Image Size(compressed)
.iXPPM:		resd 1	;X Pixel Per Meter
.iYPPM:		resd 1
.iClrUsed:	resd 1	;Number of used ColorQuad (0 for whole Quad)
.iClrImportant:	resd 1	;Number of Important ColorQuad.
	endstruc

	struc BMPOS2
		;FILE HEADER
.fType:		resw 1	;BM
.fSize:		resd 1	;whole file size
		resd 1	;reserved
.fOffBits:	resd 1	;offset from file top to image
		;CORE HEADER
.iSize:		resd 1	;CORE HEADER size
.iWidth:	resw 1	;Image Width in pixels
.iHeight:	resw 1	;Image Height in pixels
.iPlanes:	resw 1	;must be 1
.iBitCount:	resw 1	;BitPerPixel 1, 4, 8, 24 (and 15,16 for new OS/2 ?)
	endstruc

; B/W bmp can also have palettes. The first for 0, second for 1.

	struc CQuad
.b:	resb 1
.g:	resb 1
.r:	resb 1
	resb 1	;reserved
	endstruc


%if 0
int info_BMP(struct DLL_STRPICENV *env, int *info, int size, UCHAR *fp);
/* À®¸ù¤·¤¿¤é1 (Èó0?), ¼ºÇÔ¤·¤¿¤é0 */
int decode0_BMP(struct DLL_STRPICENV *env, int size, UCHAR *fp,
                int b_type, UCHAR *buf, int skip);
/* ¥¨¥é¡¼¥³¡¼¥É¤òÊÖ¤¹¡£¤È¤ê¤¢¤¨¤ºÈó0¤Ë¤·¤Æ¤ë¤À¤± */
env¤Ï64KB¤Î¥ï¡¼¥¯¥¨¥ê¥¢¤Ç¤¢¤ë¡£ÀèÆ¬8dw¤ÏÊÖ¤êÃÍÍÑ¤Ë¤Ê¤Ã¤Æ¤¤¤ë¡£
´øÈ¯À­¤é¤·¤¯, ¥¤¥ó¥¹¥¿¥ó¥¹ÊÑ¿ôÅª¤Ë¤Ï»È¤¨¤Ê¤¤¤è¤¦¤À¡£JPEG_init¤Ç
base_img¤òºî¤ë¤Î¤Ïdecode¤Î¤È¤­¤À¤±¤Ç¤¤¤¤¤È»×¤¦¡£
%endif

[absolute 0]	;nask¤Ê¤é[section .bss] org 0 ¤«¤Ê win32¤À¤«¤é¥À¥á¤«
bmpinfo:
.regs:		resd 4
.reteip:	resd 1
.env:		resd 1
.info:		resd 1
.size:		resd 1
.module:	resd 1
[absolute 0]
info:
.type:		resd 1	;1 for BMP, 2 for JPEG
		resd 1	;0
.width:		resd 1
.height:	resd 1
[section .text]
[global  _info_BMP]
_info_BMP:
	push ebx
	push ebp
	push esi
	push edi
	mov esi, [esp+bmpinfo.module]
	mov eax, [esp+bmpinfo.size]
	call bmpHeader
	test edi, edi
	jz .ret
	mov esi, [esp+bmpinfo.info]
	mov [esi+info.width], eax
	mov [esi+info.height], ecx
	mov [esi+info.type], edi	;=1
	dec edi
	mov [esi+info.type+4], edi	;=0
	inc edi
.ret:	mov eax, edi
	pop edi
	pop esi
	pop ebp
	pop ebx
	ret

;in: esi=module, eax=size
;ret:eax=width, ecx=hegiht, edx=paletteSize, ebx=palette
;    ebp=bpp, esi=endOfImage, edi=successFlag
bmpHeader:
	lea edx, [esi+eax]		;moduleEnd
	xor edi, edi
	push edx
	xor edx, edx

	cmp eax, byte BMP.iSize+4
	jbe ..@ret			;Ãæ·Ñ¤·¤Æ¤·¤Þ¤¦
	cmp word[esi],'BM'
	je .notMAC
	sub esi, byte -128
	add eax, byte -128
	pop ebx
	push eax
	cmp eax, byte BMP.iSize+4
..@ret:	jbe .ret
	cmp word[esi], 'BM'
	jne .ret
.notMAC:
	;;MS,OS/2 ¥Õ¥©¡¼¥Þ¥Ã¥È³ÎÇ§
	mov ecx, [esi +BMP.iSize]
	cmp ecx, byte 12			;OS/2 format.
	jne .MS
	  cmp eax, byte BMPOS2_size
	  jbe .ret				;core¥Ø¥Ã¥À¤Ê¤·
	  lea ebx, [esi+ecx+14]			;palette
	  movzx eax, word[esi+BMPOS2.iWidth]	;width
	  movzx ecx, word[esi+BMPOS2.iHeight]	;height
	  movzx ebp, word[esi+BMPOS2.iBitCount]	;bpp
	  mov dl, 3				;paletteSize
	jmp short .endif
.MS:	  cmp eax, byte BMP_size
	  jbe .ret				;info¥Ø¥Ã¥À¤Ê¤·
	  lea ebx, [esi+ecx+14]
	  sub ecx,byte 40
	  jne .ret				;unknownFormat
	  cmp ecx, [esi+BMP.iCompression]
	  jne .ret				;Compressed.
	  mov   eax, [esi+BMP.iWidth]		;width
	  mov   ecx, [esi+BMP.iHeight]		;height
	  movzx ebp, word[esi +BMP.iBitCount]	;bpp
	  mov dl, 4				;paletteSize
.endif:
	add esi, [esi +BMP.fOffBits]

	;size¤¬¾®¤µ¤¤¾ì¹çheight¤òºï¤Ã¤Æ¤Ç¤ 
	;ÆÉ¤á¤ë¤ÈÅú¤¨¤ë¤Ù¤­¤À¤í¤¦¡£º£¤Ï¥¨¥é¡¼
	push edx
	push eax
	mul ebp				;eax=width*bpp
	add eax, byte 7
	shr eax, 3			;lineSizeWithoutPudding
	mov edx, eax
	add eax, byte 3			;size<1GB¤ò²¾Äê¤·¤Æedx¤òÌµ» 
	and al, -4			;lineSizeWithPudding
	sub edx, eax			;-puddingSize
	push edx
	mul ecx
	pop edx
	add esi, eax
	add esi, edx			;ºÇ½ª¹Ô¤ÎºÇ¸å¤Ë¤Ïpudding¤¬¤Ê¤¤¤È¸«¤ë¤Ù¤­
	cmp esi, [esp+8]		;endOfModule
	pop eax
	ja .ret2
	sub esi, edx			;esi=endOfImage
	inc edi				;succeeded!
.ret2:	pop edx
.ret:	add esp, byte 4
	ret

;***************************************************************
; ¾®¤µ¤µÍ¥Àè¤Çºî¤Ã¤Æ¤¤¤ë¤¬, Â®¤µÍ¥Àè¤Çºî¤Ã¤¿ÀÎ¤Î¤è¤ê¤¤¤¤¤«¤â¡£
; ÉÊ¼ÁºÇÄã¤Î¹âÂ®¥â¡¼¥É¤Î¤ßºî¤Ã¤Æ¤¤¤ 

[absolute 0]
decode:
.regs:		resd 4
.reteip:	resd 1
.env:		resd 1
.size:		resd 1
.module:	resd 1
.outputType:	resd 1
.dest:		resd 1
.skip:		resd 1
[section .text]
[global _decode0_BMP]
_decode0_BMP:
	push ebx
	push ebp
	push esi
	push edi
	mov esi, [esp+decode.module]
	mov eax, [esp+decode.size]
	call bmpHeader
	;ret:eax=width, ecx=hegiht, edx=paletteSize, ebx=palette
	;    ebp=bpp, esi=endOfImage, edi=successFlag
	test edi,edi
	jz .error
	mov edi, [esp+decode.dest]
	push dword[esp+decode.outputType]
	push dword[esp+4+decode.skip]
	push ecx			;height
	push eax
	push edx
	mul ebp
	add eax, byte 31
	shr eax, 3
	and al, -4
	push eax
	mov edx, ebp
	mov ebp, esp
	call bmp2beta			;ecx!=0 for error
	add esp, byte bb.size
	mov eax, ecx
	test ecx, ecx
	jz .ret
.error:	push byte 1
	pop eax
.ret:	pop edi
	pop esi
	pop ebp
	pop ebx
	ret

[absolute -4*2]
bb:
.col0:		resd 1	;bpp1¤Ç»È¤¦
.reteip:	resd 1
.sw:		resd 1	;byte
.paletteSize:	resd 1	;byte
.w:		resd 1	;pixel
.h:		resd 1
.s:		resd 1
.t:		resd 1
.size: equ $-$$
[section .text]
;eax=?, ecx=height, edx=bpp, ebx=palette
;ebp=bb, esi=endOfImage, edi=dest
bmp2beta:
	mov al, [ebp+bb.t]
	and al, 0x7f
	cmp al, 2
	je near buf16
	cmp al, 4
	je buf32
	mov ecx, esp		;!=0
	ret
;===============================================================
;	Buffer mode 4
;===============================================================
buf32:
	dec edx
	je near .bpp1
	sub edx, byte 4-1
	je .bpp4
	sub edx, byte 8-4
	je .bpp8
	sub edx, byte 24-8
	je .bpp24
	mov ecx, esp		;!=0
	ret
;---------------------------------------------------
;	24bpp BMP to 4byte bufer
;---------------------------------------------------
.bpp24:
	;ecx=height, edx=0, ebx=palette
	;esi=endOfImage, edi=destinationBuffer
	;bb.w=width(pixel), bb.s=skipByte, bb.h=height
	;bb.t=outputType, bb.sw=sourceWidthByte

	.do24.1:
	  sub esi, [ebp+bb.sw]	;esi=startOfLine
	  push ecx
	  push esi
	  mov ecx, [ebp+bb.w]
	  .do24.2:
	    mov al, [esi]
	     mov [edi+3], dl
	    mov [edi], al
	     mov al, [esi+1]
	    mov [edi+1], al
	     mov al, [esi+2]
	    mov [edi+2], al
	     add esi, byte 3
	    add edi, byte 4
	     dec ecx
	  jnz .do24.2
	  pop esi
	  pop ecx
	  add edi, [ebp+bb.s]
	  dec ecx
	jnz .do24.1
	ret

;---------------------------------------------------
;	8bpp BMP to 4byte buffer
;---------------------------------------------------
.bpp8:
	;ecx=height, edx=0, ebx=palette
	;esi=endOfImage, edi=destinationBuffer

	;palleteÊÑ´¹
	mov dl, 255
	mov eax, [ebp+bb.paletteSize]
	sub ebx, eax
	shl eax, 8
	add ebx, eax		;ebx += paletteSize*255
	.do8.1:
	  mov eax, [ebx]
	  sub ebx, [ebp+bb.paletteSize]
	  and eax, 0x00ffffff
	  dec edx
	  push eax
	jns .do8.1

	.do8.2:
	  sub esi, [ebp+bb.sw]	;esi=firstLineStart
	  push ecx
	  push esi
	  mov ecx, [ebp+bb.w]
	  .do8.3:
	    xor eax, eax
	     add edi, byte 4
	    mov al, [esi]
	     inc esi
	    ;AGI stole
	    mov eax, [esp+eax*4+8]
	     dec ecx
	    mov [edi-4], eax
	  jnz .do8.3
	  pop esi
	  pop ecx
	  add edi, [ebp+bb.s]
	  dec ecx
	jnz .do8.2
	add esp, 256*4		;palette
	ret

;---------------------------------------------------
;	4bpp BMP to 4byte buffer
;---------------------------------------------------
.bpp4:
	;ecx=height, edx=0, ebx=palette
	;esi=endOfImage, edi=destinationBuffer

	;palleteÊÑ´¹
	mov dl, 16
	mov eax, [ebp+bb.paletteSize]
	sub ebx, eax
	shl eax, 4
	add ebx, eax			;ebx+=eax*15
	.do4.1
	  mov eax, [ebx]
	  sub ebx, [ebp+bb.paletteSize]
	  and eax, 0x00ffffff
	  dec edx
	  push eax
	jnz .do4.1

	.do4.2:
	  sub esi, [ebp+bb.sw]	;esi=firstLineStart
	  push ecx
	  push esi
	  mov ecx, [ebp+bb.w]
	  .do4.3:
	    xor edx, edx
	     mov al, [esi]
	    mov dl, al
	     inc esi
	    shr dl, 4
	     and eax, byte 15
	    add edi, byte 4
	     dec ecx
	    mov edx, [esp+edx*4+8]
	     mov eax, [esp+eax*4+8]
	    mov [edi-4], edx
	     jz .wend
	    mov [edi], eax
	     add edi, byte 4
	    dec ecx
	  jnz .do4.3
.wend:	  pop esi
	  pop ecx
	  add edi, [ebp+bb.s]
	  dec ecx
	jnz .do4.2
	add esp, 16*4		;palette
	ret

;---------------------------------------------------
;	1bpp BMP to 4byte buffer
;---------------------------------------------------
.bpp1:
	;ecx=height, edx=0, ebx=palette
	;esi=endOfImage, edi=destinationBuffer

	;palleteÊÑ´¹
	mov eax, [ebx]
	add ebx, [ebp+bb.paletteSize]
	and eax, 0x00ffffff
	mov ebx, [ebx]
	and ebx, 0x00ffffff
	xor ebx, eax
	;push ebx
	push eax

	.do1.1:
	  sub esi, [ebp+bb.sw]	;esi=firstLineStart
	  push ecx
	  push esi
	  mov ecx, [ebp+bb.w]
	  .do1.2:
	    mov dl, [esi]
	     inc esi
	    push esi
	     mov esi, 8
	    .do1.3:
	      add edi, byte 4
	       add dl, dl
	      sbb eax, eax
	      and eax, ebx
	      xor eax, [ebp+bb.col0]
	       dec ecx
	      mov [edi-4], eax
	       jz .wend1bpp
	      dec esi
	    jnz .do1.3
	    pop esi
	  jmp short .do1.2
.wend1bpp:pop ecx
	  pop esi
	  pop ecx
	  add edi, [ebp+bb.s]
	  dec ecx
	jnz .do1.1
	pop eax
	ret

;===============================================
;	Buffer mode 2byte
;===============================================
buf16:
	dec edx
	je near .bpp1
	sub edx, byte 4-1
	je near .bpp4
	sub edx, byte 8-4
	je .bpp8
	sub edx, byte 24-8
	je .bpp24
	mov ecx, esp
	ret
;---------------------------------------------------
;	24bpp BMP to 2byte bufer
;---------------------------------------------------
.bpp24:
	;ecx=height, edx=0, ebx=palette
	;esi=endOfImage, edi=destinationBuffer

	.do24.1:
	  sub esi, [ebp+bb.sw]	;esi=startOfLine
	  push ecx
	  push esi
	  mov ecx, [ebp+bb.w]
	  .do24.2:
	    mov al, [esi+2]
	    shl eax, 16
	    mov ax, [esi]
	     add esi, byte 3
	    ;¸º¿§½èÍý eax=24bitColor, edx=work, ecx=counter, ebx=work
	    ;Àî¹ç¤µ¤ó¤Î¼ñÌ£¤ÇË×¤Ã¤¿¥ë¡¼¥Á¥ó¤ò»ý¤Ã¤Æ¤¯¤ë¤â¤è¤·(¤©
	    ;¸íº¹³È»¶¥ë¡¼¥Á¥ó¤ò»ý¤Ã¤Æ¤¯¤ë¤â¤è¤·
	    shr ah, 2		;???????? RRRRRrrr 00GGGGGG BBBBBbbb 
	     inc edi
	    shr eax, 3		;000????? ???RRRRR rrr00GGG GGGBBBBB
	    shl ax, 5		;000????? ???RRRRR GGGGGGBB BBB00000
	     inc edi
	    shr eax, 5		;00000000 ???????? RRRRRGGG GGGBBBBB
	     dec ecx
	    mov [edi-2], ax
	  jnz .do24.2
	  pop esi
	  pop ecx
	  add edi, [ebp+bb.s]
	  dec ecx
	jnz .do24.1
	ret

;---------------------------------------------------
;	8bpp BMP to 2byte buffer
;---------------------------------------------------
.bpp8:
	;ecx=height, edx=0, ebx=palette
	;esi=endOfImage, edi=destinationBuffer

	;palleteÊÑ´¹
	mov dl, 255
	mov eax, [ebp+bb.paletteSize]
	sub ebx, eax
	shl eax, 8
	add ebx, eax		;ebx += paletteSize*255
	.do8.1:
	  mov eax, [ebx]
	  sub ebx, [ebp+bb.paletteSize]
	  call .paletteConv
	  dec edx
	  push eax
	jns .do8.1

	.do8.2:
	  sub esi, [ebp+bb.sw]	;esi=firstLineStart
	  push ecx
	  push esi
	  mov ecx, [ebp+bb.w]
	  .do8.3:
	    xor eax, eax
	     add edi, byte 2
	    mov al, [esi]
	     inc esi
	    ;AGI stole
	    mov eax, [esp+eax*4+8]
	     dec ecx
	    mov [edi-2], ax
	  jnz .do8.3
	  pop esi
	  pop ecx
	  add edi, [ebp+bb.s]
	  dec ecx
	jnz .do8.2
	add esp, 256*4		;palette
	ret

;---------------------------------------------------
;	4bpp BMP to 2byte buffer
;---------------------------------------------------
.bpp4:
	;ecx=height, edx=0, ebx=palette
	;esi=endOfImage, edi=destinationBuffer

	;palleteÊÑ´¹
	mov dl, 16
	mov eax, [ebp+bb.paletteSize]
	sub ebx, eax
	shl eax, 4
	add ebx, eax			;ebx+=eax*15
	.do4.1:
	  mov eax, [ebx]
	  sub ebx, [ebp+bb.paletteSize]
	  call .paletteConv
	  dec edx
	  push eax
	jnz .do4.1

	.do4.2:
	  sub esi, [ebp+bb.sw]	;esi=firstLineStart
	  push ecx
	  push esi
	  mov ecx, [ebp+bb.w]
	  .do4.3:
	    xor edx, edx
	     mov al, [esi]
	    mov dl, al
	     inc esi
	    shr dl, 4
	     and eax, byte 15
	    add edi, byte 2
	     dec ecx
	    mov edx, [esp+edx*4+8]
	     mov eax, [esp+eax*4+8]
	    mov [edi-2], dx
	     jz .wend
	    mov [edi], ax
	     add edi, byte 2
	    dec ecx
	  jnz .do4.3
.wend:	  pop esi
	  pop ecx
	  add edi, [ebp+bb.s]
	  dec ecx
	jnz .do4.2
	add esp, 16*4		;palette
	ret

;---------------------------------------------------
;	1bpp BMP to 2byte buffer
;---------------------------------------------------
.bpp1:
	;ecx=height, edx=0, ebx=palette
	;esi=endOfImage, edi=destinationBuffer

	;palleteÊÑ´¹
	mov eax, [ebx]
	add ebx, [ebp+bb.paletteSize]
	call .paletteConv
	push eax
	mov eax, [ebx]
	call .paletteConv
	pop ebx
	xchg eax, ebx
	xor ebx, eax
	push eax

	.do1.1:
	  sub esi, [ebp+bb.sw]	;esi=firstLineStart
	  push ecx
	  push esi
	  mov ecx, [ebp+bb.w]
	  .do1.2:
	    mov dl, [esi]
	     inc esi
	    push esi
	     mov esi, 8
	    .do1.3:
	      add dl, dl
	       inc edi
	      sbb eax, eax
	       inc edi
	      and eax, ebx
	      xor eax, [ebp+bb.col0]
	       dec ecx
	      mov [edi-2], ax
	       jz .wend1bpp
	      dec esi
	    jnz .do1.3
	    pop esi
	  jmp short .do1.2
.wend1bpp:
	  pop ecx
	  pop esi
	  pop ecx
	  add edi, [ebp+bb.s]
	  dec ecx
	jnz .do1.1
	pop eax
	ret

.paletteConv:
	shr ah, 2
	shr eax, 3
	shl ax, 5
	shr eax, 5
	ret
