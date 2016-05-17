[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api014.nas"]

		GLOBAL	_api_closewin

[SECTION .text]

_api_closewin:		; void api_closewin(int win);
		PUSH	EBX
		MOV		EDX,14
		MOV		EBX,[ESP+8]	; win
		INT		0x40
		POP		EBX
		RET
