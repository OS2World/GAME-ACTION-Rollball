/***********************************************************************\
 *                               RollBall                              *
 * an OS/2 2.0 PM Game                                                 *
\***********************************************************************/

#define INCL_WIN
#define INCL_WINSYS
#define INCL_WINPOINTERS
#define INCL_DEV
#define INCL_WINDIALOGS

#define INCL_DOSPROCESS

#include <os2.h>			/* PM header files */
#include <stdlib.h>
#include "Error.h"
#include "RollBall.h"

MRESULT EXPENTRY BallWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY AboutDialog(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ScoreDialog(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

HAB	hab;				/* PM anchor block handle */
PSZ     pszErrMsg;                      /* Error message */
TID     tidDT;                          /* Thread ID of drawing thread */

int	AreaX;                          /* X Position of game area */
int	AreaY;                          /* Y Position of game area */
int	AreaSizeX;                      /* Size of game area width in units */
int	AreaSizeY;                      /* Size of game area height in units */
BOOL	AreaActive=FALSE;		/* Game area active or not */

HWND	hwndFrame=NULLHANDLE;		/* Handle of frame window */
HWND	hwndClient=NULLHANDLE;		/* Handle of client window */

HPOINTER	hptrCrossHair;		/* Pointer handle */

HDC	hdcMem;
HPS	hpsMem;
HBITMAP	hbmMem;

int main(void)
{
DEVOPENSTRUC    DcData={NULL,"DISPLAY",NULL,NULL,NULL,NULL,NULL,NULL,NULL};
SIZEL           sizellogo={81,81};
HMQ     hmq;                            /* Handle of message queue */
QMSG    qmsg;                           /* Message queue */
ULONG   flCreate = FCF_SYSMENU |        /* Frame creation control flags */
            FCF_MINBUTTON | FCF_ICON | FCF_MENU |
            FCF_TASKLIST | FCF_TITLEBAR | FCF_ACCELTABLE;

if((hab=WinInitialize(0))==0L)          /* Initialize window */
    GEN_ERR(hab,hwndFrame,hwndClient);

Draw_Initialize();                      /* Initialize data of drawing thread */

if((hdcMem=DevOpenDC(hab,               /* Open memory device context for startup
                                           OS/2 logo */
    OD_MEMORY,
    (PSZ)"*",
    8L,
    (PDEVOPENDATA)&DcData,
    (HDC)NULL))==0L)
    GEN_ERR(hab,hwndFrame,hwndClient);

if((hpsMem=GpiCreatePS(hab,             /* Generate a presenation space for startup logo */
    hdcMem,
    &sizellogo,
    GPIA_ASSOC|PU_PELS))==0L)
    GEN_ERR(hab,hwndFrame,hwndClient);

if((hbmMem=GpiLoadBitmap(hpsMem,        /* Load startup logo from ressource (EXE file) */
    0L,
    OS2LOGO,
    0L,
    0L))==0L)
    GEN_ERR(hab,hwndFrame,hwndClient);

if((hmq=WinCreateMsgQueue(hab,0))==0L)  /* Initialize message queue */
    GEN_ERR(hab,hwndFrame,hwndClient);

if(!WinRegisterClass(                   /* Register window class */
    hab,                                /* Handle of anchor block */
    (PSZ)"BallWindow",                  /* Window class name */
    (PFNWP)BallWindowProc,              /* Address of window procedure */
    CS_SIZEREDRAW,                      /* Class style */
    0))                                 /* No extra window words */
     GEN_ERR(hab,hwndFrame,hwndClient);

if((hwndFrame = WinCreateStdWindow(
    HWND_DESKTOP,                       /* Desktop window is parent */
    0,                                  /* STD. window styles */
    &flCreate,                          /* Frame control flag */
    "BallWindow",                       /* Client window class name */
    "",                                 /* No window text */
    0,                                  /* No special class style */
    (HMODULE)0L,                        /* Resource is in .EXE file */
    ID_WINDOW,                          /* Frame window identifier */
                                        /* Client window handle */
    &hwndClient))==0L) GEN_ERR(hab,hwndFrame,hwndClient);

                                        /* Set window title and center it */
WinSetWindowText(hwndFrame,"RollBall - A Shareware Game for OS/2 2.0");
AreaSizeY=WinQuerySysValue(HWND_DESKTOP,SV_CYTITLEBAR);
AreaSizeY+=WinQuerySysValue(HWND_DESKTOP,SV_CYMENU);
AreaSizeX=RB_SIZE*(RB_X-2);
AreaSizeY+=RB_SIZE*(RB_Y-2)+2;
AreaX=(WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN)/2)-(AreaSizeX/2);
AreaY=(WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN)/2)-(AreaSizeY/2);

if(!WinSetWindowPos(hwndFrame,          /* Shows and activates frame */
    HWND_TOP,                           /* window position, */
                                        /* and size. */
    AreaX,AreaY,AreaSizeX,AreaSizeY,
    SWP_MOVE | SWP_ACTIVATE | SWP_SHOW | SWP_SIZE))
    GEN_ERR(hab,hwndFrame,hwndClient);

while(WinGetMsg(hab,&qmsg,0UL,0UL,0UL))
    WinDispatchMsg(hab,&qmsg);
WinDestroyWindow(hwndFrame);            /* Close window */
WinDestroyMsgQueue(hmq);                /* Close message queue */
WinTerminate(hab);                      /* Terminale the application */
exit(0);
}

MRESULT EXPENTRY BallWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
switch( msg )
{
case WM_CREATE:
    {
    static ULONG        ThreadArg=0;    /* Drawing thread parameters */
    static ULONG        ThreadFlags=0;  /* Create thread in running mode */
    static ULONG        StackSize=8192; /* Set thread stack size */
    static APIRET       rc;

    AreaActive=TRUE;                    /* Client window is active */
    hptrCrossHair=WinLoadPointer(HWND_DESKTOP,0L,ID_POINTER);
                                        /* Create drawing thread */
    if(!(rc=DosCreateThread(&tidDT,     /* Thread ID */
        (PFNTHREAD)Draw_Thread,         /* Pointer to thread */
        ThreadArg,                      /* Thread arguments */
        ThreadFlags,                    /* Thread flags */
        StackSize)))                    /* Stack size */
        DOS_ERR(rc,hwndFrame,hwndClient);
    break;
    }

case WM_USER:
    {
    HPS                 hps;
    POINTL              bmarray[4];

                                        /* Now load startup logo bitmap */
    bmarray[0].x=(RB_SIZE*(RB_X-2)/2-40); bmarray[0].y=(RB_SIZE*(RB_Y-2)/2-40);
    bmarray[1].x=bmarray[0].x+81; bmarray[1].y=bmarray[0].y+81;
    bmarray[2].x=bmarray[2].y=0; bmarray[3].x=bmarray[3].y=80;
    hps=WinGetPS(hwnd);
    GpiSetBitmap(hpsMem,hbmMem);
    if(!GpiSetBitmap(hps,
        hbmMem)) GEN_ERR(hab,hwndFrame,hwndClient);
    if(!GpiBitBlt(hps,
        hpsMem,
        4L,
        (PPOINTL)bmarray,
        ROP_SRCCOPY,
        BBO_IGNORE)) GEN_ERR(hab,hwndFrame,hwndClient);
    WinReleasePS(hps);
    break;
    }
case WM_BUTTON1DOWN:
    {
                                        /* Set focus */
    if(!WinSetFocus(HWND_DESKTOP,hwnd)) GEN_ERR(hab,hwndFrame,hwndClient);
                                        /* Send lbutton with mouse position in device
                                           coordinates, which are in mp1, but only if
                                           RollBall is rolling */
    if(runRB==TRUE) WinPostQueueMsg(hmqDT,
        DT_LBUTTON,
        MPFROMLONG(SHORT1FROMMP(mp1)),
        MPFROMLONG(SHORT2FROMMP(mp1)));
    break;
    }

case WM_BUTTON2DOWN:
    {
                                        /* Set focus */
    if(!WinSetFocus(HWND_DESKTOP,hwnd)) GEN_ERR(hab,hwndFrame,hwndClient);
                                        /* Send rbutton with mouse position in device
                                           coordinates, which are in mp1, but only if
                                           RollBall is rolling */
    if(runRB==TRUE) WinPostQueueMsg(hmqDT,
        DT_RBUTTON,
        MPFROMLONG(SHORT1FROMMP(mp1)),
        MPFROMLONG(SHORT2FROMMP(mp1)));
    break;
    }

case WM_FOCUSCHANGE:
    {
    static BOOL runRBsave=2;

    if(SHORT1FROMMP(mp2)==TRUE)         /* TRUE if receiving focus */
        {
        WinSetPointer(HWND_DESKTOP,hptrCrossHair);
                                        /* Restore RollBall's rolling indicator, but only
                                           if it is valid */
        if(runRBsave!=2) runRB=runRBsave;
        }

    if(SHORT1FROMMP(mp2)==FALSE)        /* FALSE if losing focus */
                                        /* We should stop RollBall from rolling only, if
                                           a window is gaining focus, that is not a child
                                           of our frame window */
        if((WinIsChild(HWNDFROMMP(mp1),hwndFrame)==FALSE))
            {
            runRBsave=runRB;
            runRB=FALSE;                /* If switching to another window stop RollBall */
            }
    return((MRESULT)WinDefWindowProc(hwnd,msg,mp1,mp2));
    break;
    }

case WM_MOUSEMOVE:
    {
    if(AreaActive==TRUE)
        WinSetPointer(HWND_DESKTOP,hptrCrossHair);
    break;
    }

case WM_MINMAXFRAME:                    /* Adjust titlebar text according to minimize/
                                           restore */
    {
    static BOOL runRBsave=FALSE;
                                        /* If size is smaller than the playing ground is
                                           set, then it must be minimized, because the
                                           playing ground has no sizing border */
    if(((PSWP)mp1)->fl & SWP_MINIMIZE)
        {
        WinSetWindowText(hwndFrame,"RollBall");
                                        /* If RollBall is rolling, stop it */
        if((runRBsave=runRB)==TRUE) runRB=FALSE;
        }
    else
        {
        WinSetWindowText(hwndFrame,"RollBall - A Shareware Game for OS/2 2.0");
        runRB=runRBsave;                /* Restore rolling indicator to state before
                                           minimized */
        }
    return((MRESULT)WinDefWindowProc(hwnd,msg,mp1,mp2));
    break;
    }

case WM_COMMAND:
    {
    USHORT command;                     /* WM_COMMAND command value */

    command=SHORT1FROMMP(mp1);          /* Extract the command value */
    switch (command)
        {
    case ID_ABOUT:                      /* About RollBall dialog box selected */
        {
        BOOL    runRBsave;
                                        /* If RollBall is rolling, stop it */
        if((runRBsave=runRB)==TRUE) runRB=FALSE;
        if(!WinDlgBox(HWND_DESKTOP,     /* Place on DESKTOP */
            hwndFrame,                  /* Ownd by Frame window */
            AboutDialog,                /* Implementing procedure */
            (HMODULE)0,                 /* Resouce in EXE file */
            ID_ABOUTDIALOG,             /* Dialog identifier in ressource */
                                        /* No initialization data */
            NULL)) GEN_ERR(hab,hwndFrame,hwndClient);
        runRB=runRBsave;                /* Restore RollBall rolling indicator */
        break;
        }
    case ID_SCORE:                      /* About Score dialog box selected */
        {
        BOOL    runRBsave;
                                        /* If RollBall is rolling, stop it */
        if((runRBsave=runRB)==TRUE) runRB=FALSE;
        if(!WinDlgBox(HWND_DESKTOP,     /* Place on DESKTOP */
            hwndFrame,                  /* Ownd by Frame window */
            ScoreDialog,                /* Implementing procedure */
            (HMODULE)0,                 /* Resouce in EXE file */
            ID_SCOREDIALOG,             /* Dialog identifier in ressource */
                                        /* No initialization data */
            NULL)) GEN_ERR(hab,hwndFrame,hwndClient);
        runRB=runRBsave;                /* Restore RollBall rolling indicator */
        break;
        }
    case ID_STARTTHREAD:                /* Start the game */
        {
        HWND    hwndMenu;
                                        /* Get window handle of menu bar */
        if((hwndMenu=WinWindowFromID(hwndFrame,FID_MENU))==NULLHANDLE)
            GEN_ERR(hab,hwndFrame,hwndClient);
        if((WinSendMsg(hwndMenu,        /* Disable start item */
            MM_SETITEMATTR,
            MPFROM2SHORT(ID_STARTTHREAD,TRUE),
            MPFROM2SHORT(MIA_DISABLED,MIA_DISABLED)))==FALSE)
            GEN_ERR(hab,hwndFrame,hwndClient);
        if((WinSendMsg(hwndMenu,        /* Enable pause item */
            MM_SETITEMATTR,
            MPFROM2SHORT(ID_PAUSETHREAD,TRUE),
            MPFROM2SHORT(MIA_DISABLED,0)))==FALSE)
            GEN_ERR(hab,hwndFrame,hwndClient);
        if((WinSendMsg(hwndMenu,        /* Enable stop item */
            MM_SETITEMATTR,
            MPFROM2SHORT(ID_STOPTHREAD,TRUE),
            MPFROM2SHORT(MIA_DISABLED,0)))==FALSE)
            GEN_ERR(hab,hwndFrame,hwndClient);
        Playground_Initialize();        /* Initialize playing ground with 50 symbols */
        WinInvalidateRegion(hwnd,NULLHANDLE,FALSE);
        runRB=TRUE;                     /* RollBall is rolling */
        break;
        }
    case ID_STOPTHREAD:                 /* Stop the game */
        {
        HWND            hwndMenu;
        unsigned char   MsgBuffer[512];

                                        /* Get window handle of menu bar */
        if((hwndMenu=WinWindowFromID(hwndFrame,FID_MENU))==NULLHANDLE)
            GEN_ERR(hab,hwndFrame,hwndClient);
        if((WinSendMsg(hwndMenu,        /* Disable the stop item */
            MM_SETITEMATTR,
            MPFROM2SHORT(ID_STOPTHREAD,TRUE),
            MPFROM2SHORT(MIA_DISABLED,MIA_DISABLED)))==FALSE)
            GEN_ERR(hab,hwndFrame,hwndClient);
        if((WinSendMsg(hwndMenu,        /* Disable the pause item */
            MM_SETITEMATTR,
            MPFROM2SHORT(ID_PAUSETHREAD,TRUE),
            MPFROM2SHORT(MIA_DISABLED,MIA_DISABLED)))==FALSE)
            GEN_ERR(hab,hwndFrame,hwndClient);
        if((WinSendMsg(hwndMenu,        /* Enable the start item */
            MM_SETITEMATTR,
            MPFROM2SHORT(ID_STARTTHREAD,TRUE),
            MPFROM2SHORT(MIA_DISABLED,0)))==FALSE)
            GEN_ERR(hab,hwndFrame,hwndClient);
        runRB=FALSE;                    /* RollBall is not rolling */
        sprintf(MsgBuffer,"Thank you for playing RollBall!\n"\
            "Your score is %d point(s)!\nHope you'd a good time!\n\n"\
            "If you find this game valuable, or you have any suggestions to "\
            "improve this game, let me know. (Please see RollBall.doc for "\
            "details)",RB_Point[0]);
        WinMessageBox(HWND_DESKTOP,     /* Inform the user about the points */
            hwndFrame,                  /* Ownder window */
            (PSZ)MsgBuffer,             /* Message */
            "RollBall Score Info",      /* Title bar message */
            61324,                      /* Message identifier (is random and unique) */
            MB_INFORMATION | MB_OK);
        break;
        }
    case ID_PAUSETHREAD:                /* Pause/unpause the game */
        {
        HWND            hwndMenu;
        static int      pauseOn=FALSE;
        MRESULT         rc;
                                        /* Get window handle of menu bar */
        if((hwndMenu=WinWindowFromID(hwndFrame,FID_MENU))==NULLHANDLE)
            GEN_ERR(hab,hwndFrame,hwndClient);
        if(pauseOn==TRUE)
            {
            runRB=TRUE;                 /* RollBall should be rolling */
            rc=WinSendMsg(hwndMenu,     /* Change unpause item to pause */
                MM_SETITEMTEXT,
                MPFROMLONG(ID_PAUSETHREAD),
                MPFROMP("~Pause Game"));
            if(rc==FALSE) USR_ERR("MM_SETITEMTEXT failed\n");
            GEN_ERR(hab,hwndFrame,hwndClient);
            }
        else
            {
            runRB=FALSE;                /* RollBall should be rolling */
            rc=WinSendMsg(hwndMenu,     /* Change to pause item to unpause */
                MM_SETITEMTEXT,
                MPFROMLONG(ID_PAUSETHREAD),
                MPFROMP("~Unpause Game"));
            if(rc==FALSE) USR_ERR("MM_SETITEMTEXT failed\n");
            GEN_ERR(hab,hwndFrame,hwndClient);
            }
        pauseOn=(pauseOn==TRUE ? FALSE : TRUE);
        break;
        }
    case ID_EXITPROGRAM:
        WinPostMsg(hwnd,WM_CLOSE,(MPARAM)0,(MPARAM)0);
        break;
    default:
        return((MRESULT)WinDefWindowProc(hwnd,msg,mp1,mp2));
        }
    break;
    }

case WM_PAINT:
    {
    static BOOL         logoinit=TRUE;
    static HPS          hps;		/* Handle of a presentaion space */

                                        /* Create a presentation space, even if don't
                                           paint here, this call prevents us that endless
                                           WM_PAINT messages are generated  */
    hps = WinBeginPaint(hwnd,
        NULLHANDLE,                     /* Presentation space (a chached) */
        NULL);                          /* Bounding rectangle (none, no painting required) */
                                        /* Send paint to drawing thread */
                                        /* Repaint client window aread */
    WinPostQueueMsg(hmqDT,DT_PAINT,0UL,0UL);
    WinEndPaint(hps);                   /* Release presenation space */
    if(logoinit==TRUE)                  /* Display logo only at startup */
        {
        logoinit=FALSE;                 /* Disable logo for further repaints */
        DosSleep(50);                   /* Force pre-emting to second thread to clear
                                           window to white */
                                        /* Display startup logo */
        WinPostMsg(hwnd,WM_USER,NULL,NULL);
        }
    break;
    }
case WM_CLOSE:
                                        /* Post exit to drawing thread */
    WinPostQueueMsg(hmqDT,DT_EXIT,0UL,0UL);
                                        /* Wait for shutdown of drawing thread */
    DosWaitThread(&tidDT,DCWW_WAIT);
    WinPostMsg(hwnd,WM_QUIT,NULL,NULL);
    break;

default:
    return((MRESULT)WinDefWindowProc(hwnd, msg, mp1, mp2));
}
return((MRESULT)FALSE);
}

/***********************************************************************\
 *                               RollSub1                              *
 * About Dialog routine for RollBall                                   *
\***********************************************************************/

MRESULT EXPENTRY AboutDialog(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
switch(msg)
{
case WM_COMMAND:                        /* Clear About dialog box */
    WinDismissDlg(hwnd, TRUE);
    break;
default:
    return(WinDefDlgProc(hwnd,msg,mp1,mp2));
}
return((MRESULT)0);
}

/***********************************************************************\
 *                               RollSub1                              *
 * Score Dialog routine for RollBall                                   *
\***********************************************************************/

MRESULT EXPENTRY ScoreDialog(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
switch(msg)
{
case WM_COMMAND:                        /* Clear Score dialog box */
    WinDismissDlg(hwnd, TRUE);
    break;
default:
    return(WinDefDlgProc(hwnd,msg,mp1,mp2));
}
return((MRESULT)0);
}

