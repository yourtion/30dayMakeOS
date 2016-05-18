[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api026.nas"]

		GLOBAL	_api_cmdline

[SECTION .text]

_api_cmdline:		; int api_cmdline(char *buf, int maxsize);
		PUSH	EBX
		MOV		EDX,26
		MOV		ECX,[ESP+12]		; maxsize
		MOV		EBX,[ESP+8]			; buf
		INT		0x40
		POP		EBX
		RET
