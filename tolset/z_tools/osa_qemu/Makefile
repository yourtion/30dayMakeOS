MAKE     = ..\make.exe -r
OSALINK  = ..\osalink1.exe 
MV       = ren
EDIMG    = ..\edimg.exe
RM       = del

ALL :
	$(MAKE) osaimgqe.bin

osask_qe.exe : Makefile OSASK.EXE timerdrv.tek osalink1.opt
	-$(RM) osask_qe.exe
	$(MV) OSASK.EXE osask_at.exe
	$(OSALINK)
	$(MV) osask.exe osask_qe.exe
	$(MV) osask_at.exe OSASK.EXE

osask_qe.sys : Makefile osask_qe.exe
	$(EDIMG) binin:osask_qe.exe exe2bin seg:2048 binout:osask_qe.sys

osaimgqe.bin : Makefile OSAIMGAT.BIN osask_qe.sys
	$(EDIMG) @edimgopt.txt
