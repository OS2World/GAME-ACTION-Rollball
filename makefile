#===================================================================
#
#   RollBall Makefile
#
#===================================================================

# ICC Flags
#	/Ti	Include debug information for IPMD
#	/Kb+	Warning, if no prototypes found (prevents from incorrect params)
#	/c	Compile only, we link more than one ressource
#	/Se	Allow IBM C language extentions and migration
#	/Re	Produce Code in IBM C Set/2 Run Time environment
#	/ss	Allow // as comments
#	/Gm+	Link with multitasking libraries, because we're multithreaded	
CFLAGS	= /Ti /Kb+ /c /Se /Re /ss /Gm+

# LINK386 Flags
#	/DEBUG	Include debug information for IPMD
#	/NOE	NO Extented dictionary, don't assume any library dependencies
#	/NOD	NO Default library, ignore the library names in object files
#	/A:16	Align on paragraph for PM programs
#	/M	Produce map
#	/BASE	Base over 1Meg ?
LFLAGS	= /DEBUG /NOE /NOD /ALIGN:16 /M /BASE:0x10000

# Libraries
#	DD4MBS	Multitasking standard library
#	OS2386	OS/2 2.0 Link library
LIBS	= DDE4MBS + OS2386
CC	= icc $(CFLAGS)
LINK	= LINK386 $(LFLAGS)

HEADERS = RollBall.h Error.h

ALL_OBJ1 = RollBall.obj Error.obj Thread.obj
# RollSub1.obj 

all: RollBall.exe

clean:
	del *.exe
	del *.obj
	del *.res

save:
	pkzip RollBall *
	pkzip -d RollBall *.zip
	copy RollBall.zip RollSave.zip /v

Error.obj: Error.c Error.h
	$(CC) Error.c

Thread.obj: Thread.c Error.h
    $(CC) Thread.c

RollBall.l: Makefile
	echo $(ALL_OBJ1)  > RollBall.l
	echo RollBall.exe >> RollBall.l
	echo RollBall.map >> RollBall.l
	echo $(LIBS)      >> RollBall.l
	echo RollBall.def >> RollBall.l

RollBall.res: RollBall.rc RollBall.ico RollBall.h
	rc -r RollBall.rc

RollBall.obj: RollBall.c $(HEADERS)
	$(CC) RollBall.c

#RollSub1.obj: RollSub1.c $(HEADERS)
#	$(CC) RollSub1.c

RollBall.exe: $(ALL_OBJ1) RollBall.def RollBall.l RollBall.res
    $(LINK) @RollBall.l
    rc RollBall.res RollBall.exe
