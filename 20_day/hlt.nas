[BITS 32]
		MOV		AL,'A'
		CALL	2*8:0xbe3
fin:
		HLT
		JMP		fin