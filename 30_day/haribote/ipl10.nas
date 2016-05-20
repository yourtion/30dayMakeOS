; haribote-ipl
; TAB=4

CYLS	EQU		10				; 声明CYLS=10

		ORG		0x7c00			; 指明程序装载地址

; 标准FAT12格式软盘专用的代码 Stand FAT12 format floppy code

		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; 启动扇区名称（8字节）
		DW		512				; 每个扇区（sector）大小（必须512字节）
		DB		1				; 簇（cluster）大小（必须为1个扇区）
		DW		1				; FAT起始位置（一般为第一个扇区）
		DB		2				; FAT个数（必须为2）
		DW		224				; 根目录大小（一般为224项）
		DW		2880			; 该磁盘大小（必须为2880扇区1440*1024/512）
		DB		0xf0			; 磁盘类型（必须为0xf0）
		DW		9				; FAT的长度（必9扇区）
		DW		18				; 一个磁道（track）有几个扇区（必须为18）
		DW		2				; 磁头数（必2）
		DD		0				; 不使用分区，必须是0
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29		; 意义不明（固定）
		DD		0xffffffff		; （可能是）卷标号码
		DB		"HARIBOTEOS "	; 磁盘的名称（必须为11字?，不足填空格）
		DB		"FAT12   "		; 磁盘格式名称（必??8字?，不足填空格）
		RESB	18				; 先空出18字节

; 程序主体

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

; 读取磁盘

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; 柱面0
		MOV		DH,0			; 磁头0
		MOV		CL,2			; 扇区2
		MOV		BX,18*2*CYLS-1	; 要读取的合计扇区数
		CALL	readfast		; 告诉读取

; 读取完毕，跳转到haribote.sys执行！
		MOV		[0x0ff0],CH		; 记录IPL实际读取了多少内容
		JMP		0xc200

error:
		MOV		AX,0
		MOV		ES,AX
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; 给SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 显示一个文字
		MOV		BX,15			; 指定字符颜色
		INT		0x10			; 调用显卡BIOS
		JMP		putloop
fin:
		HLT						; 让CPU停止，等待指令
		JMP		fin				; 无限循环
msg:
		DB		0x0a, 0x0a		; 换行两次
		DB		"load error"
		DB		0x0a			; 换行
		DB		0

readfast:	; 使用AL尽量一次性读取数据 从此开始
; ES:读取地址, CH:柱面, DH:磁头, CL:扇区, BX:读取扇区数

		MOV		AX,ES			; < 通过ES计算AL的最大值 >
		SHL		AX,3			; 将AX除以32，将结果存入AH（SHL是左移位指令）
		AND		AH,0x7f		; AH是AH除以128所得的余数（512*128=64K）
		MOV		AL,128		; AL = 128 - AH; AH是AH除以128所得的余数（512*128=64K）
		SUB		AL,AH

		MOV		AH,BL			; < 通过BX计算AL的最大值并存入AH >
		CMP		BH,0			; if (BH != 0) { AH = 18; }
		JE		.skip1
		MOV		AH,18
.skip1:
		CMP		AL,AH			; if (AL > AH) { AL = AH; }
		JBE		.skip2
		MOV		AL,AH
.skip2:

		MOV		AH,19			; < 通过CL计算AL的最大值并存入AH >
		SUB		AH,CL			; AH = 19 - CL;
		CMP		AL,AH			; if (AL > AH) { AL = AH; }
		JBE		.skip3
		MOV		AL,AH
.skip3:

		PUSH	BX
		MOV		SI,0			; 计算失败次数的寄存器
retry:
		MOV		AH,0x02			; AH=0x02 : 读取磁盘
		MOV		BX,0
		MOV		DL,0x00			; A盘
		PUSH	ES
		PUSH	DX
		PUSH	CX
		PUSH	AX
		INT		0x13			; 调用磁盘BIOS
		JNC		next			; 没有出错的话则跳转至next
		ADD		SI,1			; 将SI加1
		CMP		SI,5			; 将SI与5比较
		JAE		error			; SI >= 5则跳转至error
		MOV		AH,0x00
		MOV		DL,0x00		; A盘
		INT		0x13			; 驱动器重置
		POP		AX
		POP		CX
		POP		DX
		POP		ES
		JMP		retry
next:
		POP		AX
		POP		CX
		POP		DX
		POP		BX				; 将ES的内容存入BX
		SHR		BX,5			; 将BX由16字节为单位转换为512字节为单位
		MOV		AH,0
		ADD		BX,AX			; BX += AL;
		SHL		BX,5			; 将BX由512字节为单位转换为16字节为单位
		MOV		ES,BX			; 相当于EX += AL * 0x20;
		POP		BX
		SUB		BX,AX
		JZ		.ret
		ADD		CL,AL			; 将CL加上AL
		CMP		CL,18			; 将CL与18比较
		JBE		readfast	; CL <= 18则跳转至readfast
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readfast	; DH < 2则跳转至readfast
		MOV		DH,0
		ADD		CH,1
		JMP		readfast
.ret:
		RET

		RESB	0x7dfe-$	; 到0x7dfe为止用0x00填充的指令

		DB		0x55, 0xaa
