[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api009.nas"]

		GLOBAL	_api_malloc

[SECTION .text]

_api_malloc:		; char *api_malloc(int size);
		PUSH	EBX
		MOV		EDX,9
		MOV		EBX,[CS:0x0020]
		MOV		ECX,[ESP+8]			; size
		INT		0x40
		POP		EBX
		RET
