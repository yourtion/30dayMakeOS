[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api019.nas"]

		GLOBAL	_api_freetimer

[SECTION .text]

_api_freetimer:		; void api_freetimer(int timer);
		PUSH	EBX
		MOV		EDX,19
		MOV		EBX,[ESP+ 8]		; timer
		INT		0x40
		POP		EBX
		RET
