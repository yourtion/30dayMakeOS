[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "crack7.nas"]

		GLOBAL	_HariMain

[SECTION .text]

_HariMain:
		MOV		AX,1005*8
		MOV		DS,AX
		CMP		DWORD [DS:0x0004],'Hari'
		JNE		fin								; 不是应用程序，因此不执行任何操作

		MOV		ECX,[DS:0x0000]		; 读取该应用程序数据段的大小
		MOV		AX,2005*8
		MOV		DS,AX

crackloop:							; 整个用123填充
		ADD		ECX,-1
		MOV		BYTE [DS:ECX],123
		CMP		ECX,0
		JNE		crackloop

fin:										; 结束
		MOV		EDX,4
		INT		0x40
