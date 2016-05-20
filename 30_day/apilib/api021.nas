[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api021.nas"]

		GLOBAL	_api_fopen

[SECTION .text]

_api_fopen:			; int api_fopen(char *fname);
		PUSH	EBX
		MOV		EDX,21
		MOV		EBX,[ESP+8]			; fname
		INT		0x40
		POP		EBX
		RET
