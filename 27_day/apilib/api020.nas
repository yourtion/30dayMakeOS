[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api020.nas"]

		GLOBAL	_api_beep

[SECTION .text]

_api_beep:			; void api_beep(int tone);
		MOV		EDX,20
		MOV		EAX,[ESP+4]			; tone
		INT		0x40
		RET
