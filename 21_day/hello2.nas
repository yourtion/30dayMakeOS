[INSTRSET "i486p"]
[BITS 32]
		MOV		EDX,2
		MOV		EBX,msg
		INT		0x40
		MOV 	EDX,4
		INT		0x40
msg:
		DB	"hello",0
