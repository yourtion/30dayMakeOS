; haribote-os
; TAB=4

		ORG		0xc200			; 知道程序被装载到内存的地址

		MOV		AL,0x13			; VGA显卡、320x200x8bit 彩色
		MOV		AH,0x00
		INT		0x10
fin:
		HLT
		JMP		fin