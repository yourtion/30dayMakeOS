[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api017.nas"]

		GLOBAL	_api_inittimer

[SECTION .text]

_api_inittimer:		; void api_inittimer(int timer, int data);
		PUSH	EBX
		MOV		EDX,17
		MOV		EBX,[ESP+ 8]		; timer
		MOV		EAX,[ESP+12]		; data
		INT		0x40
		POP		EBX
		RET
