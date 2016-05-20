[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api016.nas"]

		GLOBAL	_api_alloctimer

[SECTION .text]

_api_alloctimer:	; int api_alloctimer(void);
		MOV		EDX,16
		INT		0x40
		RET
