#===================================================================
#   RollBall - Open Watcom makefile
#===================================================================

CC      = wcc386
LINK    = wlink
RC      = wrc

CFLAGS  = -bt=os2 -mf -5 -fpi -Oaxt -W3 -ze -d0 -i=$(OS2TK)\h -i=src
RCFLAGS = -i=$(OS2TK)\h -i=src

all : bin\rollball.exe

bin\rollball.exe : bin\rollball.obj bin\thread.obj bin\error.obj bin\rollball.res
	$(LINK) system os2v2 pm option stack=65536 option map=bin\rollball.map name bin\rollball.exe file bin\rollball.obj, bin\thread.obj, bin\error.obj
	$(RC) bin\rollball.res bin\rollball.exe

bin\rollball.obj : src\rollball.c src\rollball.h src\lang.h src\error.h
	$(CC) $(CFLAGS) -fo=bin\rollball.obj src\rollball.c

bin\thread.obj : src\thread.c src\rollball.h src\error.h src\thread.h
	$(CC) $(CFLAGS) -fo=bin\thread.obj src\thread.c

bin\error.obj : src\error.c src\error.h
	$(CC) $(CFLAGS) -fo=bin\error.obj src\error.c

bin\rollball.res : src\rollball.rc src\rollball.h src\rollball.dlg
	$(RC) $(RCFLAGS) -r src\rollball.rc -fo=bin\rollball.res

clean : .SYMBOLIC
	-del bin\rollball.obj 2>NUL
	-del bin\thread.obj 2>NUL
	-del bin\error.obj 2>NUL
	-del bin\rollball.res 2>NUL
	-del bin\rollball.exe 2>NUL
	-del bin\rollball.map 2>NUL
