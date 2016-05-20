[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api015.nas"]

		GLOBAL	_api_getkey

[SECTION .text]

_api_getkey:		; int api_getkey(int mode);
		MOV		EDX,15
		MOV		EAX,[ESP+4]	; mode
		INT		0x40
		RET
