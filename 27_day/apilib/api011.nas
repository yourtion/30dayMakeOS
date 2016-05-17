[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api011.nas"]

		GLOBAL	_api_point

[SECTION .text]

_api_point:		; void api_point(int win, int x, int y, int col);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX,11
		MOV		EBX,[ESP+16]	; win
		MOV		ESI,[ESP+20]	; x
		MOV		EDI,[ESP+24]	; y
		MOV		EAX,[ESP+28]	; col
		INT		0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET
