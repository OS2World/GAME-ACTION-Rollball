/***********************************************************************\
 *                                Thread.c                             *
\***********************************************************************/

#define         INCL_DOSDEVICES
#define         INCL_DOSDEVIOCTL
#define         INCL_DOSMISC
#define         INCL_PM
#define         QSV_MS_COUNT 14

#include        <os2.h>                 /* PM header files */
#include        <stdlib.h>
#include        "Error.h"
#include        "RollBall.h"
#include        "thread.h"              /* Hi-res timer */

void    Put_Random_Field(void);
void    Clear_Random_Field(void);
void    Draw_Bitmap(ULONG Bitmap,ULONG mode,ULONG x,ULONG y);

HAB             habDT;                  /* Anchor block handle for drawing thread */
HPS             hpsDT;                  /* Handle of presentation space for window */
QMSG            qmsqDT;                 /* Message queue for drawing thread */
HMQ             hmqDT;                  /* Message queue handle for drawing thread
                                           message queue */
BOOL            runRB;                  /* RollBall should roll indicator */
                                        /* The position of RollBall RB_PosX and RB_PosY
                                           point to the lower left corner of the RollBall
                                           bitmap. To test for hits against the border, the
                                           size of the bitmap is added. To test against
                                           deflectors and points, the lower left edge of
                                           both symbols must match, which is RB_PosX*RB_SIZE
                                           and RB_PosY*RB_SIZE */
LONG            RB_PosX;                /* Position of RollBall on X-axis */
LONG            RB_PosY;                /* Position of RollBall on Y-axis */
                                        /* Direction where RollBall is rolling to, the
                                           direction changes, as RollBall hits the border,
                                           or the deflectors / and \, hitting a point only
                                           increases the score in the field RB_Point[0] */
typedef enum {RIGHT,UP,LEFT,DOWN} _RB_Dir;
_RB_Dir         RB_Dir;
ULONG           RB_Array[RB_X][RB_Y];   /* The playing ground of RollBall is an array
                                           of 40x25, whereby the surrounding fields
                                           are the playing ground borders. The array
                                           can be: */
#define         RB_EMPTY    0           /*      An empty field */
#define         RB_BP       1           /*      The blue 1 point */
#define         RB_GP       2           /*      The green 5 points */
#define         RB_MP       3           /*      The magenta 10 points */
#define         RB_VP       4           /*      The violett 20 points */
#define         RB_LX       5           /*      The left (\) deflector */
#define         RB_RX       6           /*      The right (/) deflector */
#define         RB_HOLE     7           /*      The hole, that kills a player */
#define         RB_RB       8           /*      The RollBall */
#define         RB_BORDER   9           /*      The surrounding border */

                                        /* The scoring array, where the first position 
                                           contains all points earned up to now */
ULONG           RB_Point[5]={0,1,5,10,20};
                                        /* For the following handles, the first field
                                           is unused (the field RB_Point[0]) ! */
HDC             hdcRB[RB_BORDER];       /* For each symbol prepare a device context */
HPS             hpsRB[RB_BORDER];       /* For each symbol prepare a presentation space */
HBITMAP         hbmRB[RB_BORDER];       /* For each symbol prepare a bitmap handle */
                                        /* For each symbol prepare device setup */
DEVOPENSTRUC    dcRB[RB_BORDER]={NULL,"DISPLAY",NULL,NULL,NULL,NULL,NULL,NULL,NULL};
                                        /* Size of each symbol presentation space */
SIZEL           sizelRB={RB_SIZE,RB_SIZE};
                                        /* IDs of each symbol in ressource compilition */
ULONG           RB_RESSOURCE_ID[RB_BORDER]={0,BM_BP,BM_GP,BM_MP,BM_VP,BM_LX,BM_RX,BM_HOLE,BM_RB};

HFILE           hfile=NULLHANDLE;
ULONG           ulDelay=50L;
ULONG           ulDelay2;
ULONG           ulAction=0L;
ULONG           ulSize2=sizeof(ulDelay2);


                                        /* Initialize data for our drawing thread */
int             Draw_Initialize(void)
{
int             i;                      /* Temporary variables */

                                        /* For each symbol, open a memory devicecontext,
                                           create a presentation space and load the bitmap
                                           from the EXE file */
for(i=1;i<RB_BORDER;i++)
   {
   if(!(hdcRB[i]=DevOpenDC(hab,         /* Open device context with anchor block handle of
                                           our application */
        OD_MEMORY,                      /* Type of device context (memory) */
        (PSZ)"*",                       /* Device information token (no initialization) */
        8L,                             /* Number of items (of deviceopendata) */
        (PDEVOPENDATA)&dcRB[i],         /* Open device context data */
        (HDC)NULL)))                    /* Compatible device context (compatibilty with screen) */
        GEN_ERR(hab,hwndFrame,hwndClient);
   if(!(hpsRB[i]=GpiCreatePS(hab,       /* Create a presentation space with our anchor block
                                           handle */
        hdcRB[i],                       /* Device context handle (of our symbols) */
        &sizelRB,                       /* Presentation space size (or our symbols) */
        GPIA_ASSOC|PU_PELS)))           /* Options (assoziate to screen, pels as unit) */
        GEN_ERR(hab,hwndFrame,hwndClient);
                                        /* Load bitmap from EXE file */
   if(!(hbmRB[i]=GpiLoadBitmap(hpsRB[i],
        (HMODULE)0,                     /* Ressource (EXE file) */
        RB_RESSOURCE_ID[i],             /* ID of bitmap within ressource */
        0L,                             /* Width of bitmap in pels (no streching) */
        0L)))                           /* Height of bitmap in pels (no streching) */
        GEN_ERR(hab,hwndFrame,hwndClient);
   }
return(0);
}

void            Playground_Initialize(void)
{
int     i,j,x,y;
ULONG   sys_timer;
APIRET  rc;

if((rc=DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&sys_timer,4UL))!=0)
    DOS_ERR(rc,hwndFrame,hwndClient);
srand(sys_timer);                       /* Start random generator with a really random
                                           number, the milliseconds count since bootup */
RB_Point[0]=0;                          /* Reset points */
ulDelay=50L;
for(i=0;i<=RB_X;i++)                    /* Set border lines of playground */
   RB_Array[i][0]=RB_Array[i][(RB_Y-1)]=RB_BORDER;
for(i=0;i<=RB_Y;i++)
   RB_Array[0][i]=RB_Array[(RB_X-1)][i]=RB_BORDER;
for(i=1;i<RB_X;i++)                     /* Clear the playing ground */
   for(j=1;j<RB_Y;j++)
      RB_Array[i][j]=RB_EMPTY;
for(i=1;i<50;i++)                       /* 50 random symbols */
    Put_Random_Field();
do                                      /* Position RollBall randomly on the playing
                                           ground, rolling in a random direction */
{
    x=rand()%(RB_X-2);                  /* Random position on playing ground */
    y=rand()%(RB_Y-2);
    i=rand()%4;                         /* Random direction */
    RB_PosX=x*RB_SIZE;
    RB_PosY=y*RB_SIZE;
    RB_Dir=i;
}while(RB_Array[x][y]!=RB_EMPTY);
}

void    Put_Random_Field(void)          /* Put a random symbol in a random field */
{
int     i,k,x,y;

do
   {
    x=(rand()%(RB_X-2))+1;              /* Random position on playing ground */
    y=(rand()%(RB_Y-2))+1;
    i=(rand()%136);                     /* Fill symbols with probability classes */
    if((i>=0) && (i<17)) k=RB_VP;
    if((i>=17) && (i<35)) k=RB_MP;
    if((i>=35) && (i<60)) k=RB_GP;
    if((i>=60) && (i<105)) k=RB_BP;
    if((i>=105) && (i<115)) k=RB_LX;
    if((i>=115) && (i<125)) k=RB_RX;
    if((i>=125) && (i<136)) k=RB_HOLE;
    if(RB_Array[x][y]==RB_EMPTY)        /* Add the symbol, but only if the field is empty */
        {
        RB_Array[x][y]=k;
        Draw_Bitmap(k,ROP_SRCCOPY,(x-1)*RB_SIZE,(y-1)*RB_SIZE);
        break;
        }
    }while(1);                          /* Loop until one symbol added in a random field */
}

void    Clear_Random_Field()            /* Clear a random field to RB_EMPTY. There's a
                                           great chance, that we get an empty field, so
                                           we retry 50 times */
{
int     i,x,y;

for(i=0;i<50;i++)                       /* Retry 50 times */
    {
    x=(rand()%(RB_X-2))+1;              /* Random position on playing ground */
    y=(rand()%(RB_Y-2))+1;
    if(RB_Array[x][y]!=RB_EMPTY)        /* Clear the symbol, but only if the field isn't empty */
        {
        RB_Array[x][y]=RB_EMPTY;
        Draw_Bitmap(RB_EMPTY,ROP_SRCCOPY,(x-1)*RB_SIZE,(y-1)*RB_SIZE);
        break;
        }
    }
}

void Draw_Thread(ULONG ulThreadArg)
{
if(!(habDT=WinInitialize(0UL)))         /* Initialize client window */
    GEN_ERR(habDT,hwndFrame,hwndClient);
                                        /* Create a message queue */
if(!(hmqDT=WinCreateMsgQueue(habDT,0UL)))
    GEN_ERR(habDT,hwndFrame,hwndClient);
if(!(hpsDT=WinGetPS(hwndClient)))       /* Get a presentation space for client area */
    GEN_ERR(habDT,hwndFrame,hwndClient);
                                        /* Initialize message queue */
WinPostQueueMsg(hmqDT,DT_PAINT,0UL,0UL);
while(qmsqDT.msg!=DT_EXIT)
    {
    if(WinPeekMsg(habDT,                /* Get the message into message queue */
    &qmsqDT,                            /* Message structure */
    NULLHANDLE,                         /* Window filter (none) */
    0UL,                                /* First message ID */
    0UL,                                /* Last message ID */
    PM_REMOVE)==FALSE)                  /* Options (remove message) */
        qmsqDT.msg=DT_IDLE;             /* If no message available, assume idle */
    switch(qmsqDT.msg)
    {
    case DT_PAINT:                      /* Repaint client window */
        {
        RECTL   rclDT;
        int     x,y;
                                        /* Repaint client window aread */
        WinQueryWindowRect(hwndClient,&rclDT);
        WinFillRect(hpsDT,&rclDT,CLR_WHITE);
        for(x=1;x<RB_X;x++)             /* Draw the entries on playing ground */
            for(y=1;y<RB_Y;y++)
                if(RB_Array[x][y]!=RB_EMPTY)
                    Draw_Bitmap(RB_Array[x][y],ROP_SRCCOPY,(x-1)*RB_SIZE,(y-1)*RB_SIZE);
        break;
        }
    case DT_LBUTTON:
        {
        int     x,y;
                                        /* Left button was pressed, get the location,
                                           add \ to RB_Array, and draw \ bitmap, if
                                           field is emty */
        x=(LONGFROMMP(qmsqDT.mp1)/RB_SIZE)+1;
        y=(LONGFROMMP(qmsqDT.mp2)/RB_SIZE)+1;
        if(RB_Array[x][y]==RB_EMPTY)
            {
            RB_Array[x][y]=RB_LX;
            Draw_Bitmap(RB_LX,ROP_SRCCOPY,(x-1)*RB_SIZE,(y-1)*RB_SIZE);
            }
        break;
        }
    case DT_RBUTTON:
        {
        int     x,y;
                                        /* Right button was pressed, get the location,
                                           add / to RB_Array, and draw / bitmap, if
                                           field is emty */
        x=(LONGFROMMP(qmsqDT.mp1)/RB_SIZE)+1;
        y=(LONGFROMMP(qmsqDT.mp2)/RB_SIZE)+1;
        if(RB_Array[x][y]==RB_EMPTY)
            {
            RB_Array[x][y]=RB_RX;
            Draw_Bitmap(RB_RX,ROP_SRCCOPY,(x-1)*RB_SIZE,(y-1)*RB_SIZE);
            }
        break;
        }
     case DT_IDLE:
        {
        if(runRB==TRUE)
            {
            ULONG       x,y,Symbol;
                                        /* Under DOS we would query the time in milliseconds
                                           from the system timer, to adjust graphics. This
                                           is accurate, but in a multitasking in a multitasking
                                           system, we must assume being pre-empted. Therefore
                                           we can't have an exact time bases. Hope that
                                           the system timer counts more often than all 31
                                           milliseconds in future releases/machines */
                                        /* Draw bitmap */
            switch(RB_Dir)              /* Test that RollBall doesn't leave borders. A
                                           border reverses the direction and produces a
                                           beep */
            {
            case UP:
                RB_PosY++;
                if((RB_PosY+RB_SIZE)>=((RB_Y-2)*RB_SIZE))
                    {
                    RB_PosY=(RB_Y-3)*RB_SIZE;
                    RB_Dir=DOWN;
                    DosBeep(800,50);
                    }
                break;
            case DOWN:
                RB_PosY--;
                if(RB_PosY<0)
                    {
                    RB_PosY=0;
                    RB_Dir=UP;
                    DosBeep(800,50);
                    }
                break;
            case LEFT:
                RB_PosX--;
                if(RB_PosX<0)
                    {
                    RB_PosX=0;
                    RB_Dir=RIGHT;
                    DosBeep(800,50);
                    }
                break;
            case RIGHT:
                RB_PosX++;
                if((RB_PosX+RB_SIZE)>=((RB_X-2)*RB_SIZE))
                    {
                    RB_PosX=(RB_X-3)*RB_SIZE;
                    RB_Dir=LEFT;
                    DosBeep(800,50);
                    }
                break;
            }
                                        /* Draw RollBall at new position */
            Draw_Bitmap(RB_RB,ROP_SRCCOPY,RB_PosX,RB_PosY);
                                        /* Now, test if the middle of RollBall is over any
                                           symbol. If a symbol is found, add points, deflect
                                           or end game */
                                        /* RB_Array is 1 based, because 0 indices are the
                                           playing ground borders */
            x=((RB_PosX)/RB_SIZE)+1;
            y=((RB_PosY)/RB_SIZE)+1;
                                        /* A Symbol if RB_SIZE*RB_SIZE in size, that means
                                           RollBall is exactly over a symbol, if the lower
                                           left edges of both symbols match. Then, and only
                                           then, we count points, deflect or loose */
            if((RB_PosX==(x-1)*RB_SIZE) && (RB_PosY==(y-1)*RB_SIZE))
                Symbol=RB_Array[x][y];
            else Symbol=RB_EMPTY;
            switch(Symbol)
            {
            case RB_LX:                 /* We got a \ deflector */
                {
                switch(RB_Dir)          /* \ deflects direction of RollBall */
                {
                case RIGHT:
                    RB_Dir=DOWN; break;
                case UP:
                    RB_Dir=LEFT; break;
                case LEFT:
                    RB_Dir=UP; break;
                case DOWN:
                    RB_Dir=RIGHT; break;
                }                       /* Remove deflector */
                RB_Array[x][y]=RB_EMPTY;
                break;
                }
            case RB_RX:                 /* We got a / deflector */
                {
                switch(RB_Dir)          /* / deflects direction of RollBall */
                {
                case RIGHT:
                    RB_Dir=UP; break;
                case UP:
                    RB_Dir=RIGHT; break;
                case LEFT:
                    RB_Dir=DOWN; break;
                case DOWN:
                    RB_Dir=LEFT; break;
                }                       /* Remove deflector */
                RB_Array[x][y]=RB_EMPTY;
                DosBeep(600,20);
                break;
                }
            case RB_BP:                 /* We got a point */
            case RB_GP:
            case RB_MP:
            case RB_VP:
                {                       /* Add the points for each symbol */
                RB_Point[0]+=RB_Point[Symbol];
                                        /* Remove the point */
                RB_Array[x][y]=RB_EMPTY;
                if (ulDelay)
                    ulDelay--;
                DosBeep(700,20);
                break;
                }
            case RB_HOLE:               /* We got a hole, sorry but RollBall will be killed.
                                           We disable RollBall from rolling, and send a 
                                           ID_STOPTHREAD message to our window, which
                                           informs the user about the points with a message
                                           box */
                {
                int     freq;
                for(freq=5000;freq>100;freq-=100) DosBeep(freq,5);
                runRB=FALSE;            /* Prevent RollBall from further rolling */
                WinPostMsg(hwndClient,WM_COMMAND,(MPARAM)ID_STOPTHREAD,(MPARAM)0);
                break;
                }
            }
                                        /* Randomly add and remove symbols on playing ground */
            if((rand()%500)<2)
                {
                Put_Random_Field();
                Clear_Random_Field();
                }
            }
        }
    }
DosOpen("TIMER0$",
        &hfile,
        &ulAction,
        0,
        0,
        OPEN_ACTION_OPEN_IF_EXISTS,
        OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE,
        NULL);   
ulDelay2=ulDelay/2;
DosDevIOCtl(hfile,
            HRT_IOCTL_CATEGORY,
            HRT_BLOCKUNTIL,
            &ulDelay2,
            ulSize2,
            &ulSize2,
            NULL,
            0,
            NULL);
DosClose(hfile);
}
WinReleasePS(hpsDT);                    /* Clean up */
WinDestroyMsgQueue(hmqDT);
WinTerminate(habDT);
DosExit(EXIT_THREAD,0UL);
}

                                        /* Draw a bitmap on client window */
void Draw_Bitmap(ULONG Bitmap,ULONG mode,ULONG x,ULONG y)
{
static POINTL   D_Array[5];             /* Structure to locate bitmap into window */
                                        /* Destination edges */
D_Array[0].x=x; D_Array[0].y=y; D_Array[1].x=x+RB_SIZE; D_Array[1].y=y+RB_SIZE;
                                        /* Source edges */
D_Array[2].x=D_Array[2].y=0; D_Array[3].x=D_Array[3].y=RB_SIZE;
                                        /* Select bitmap */
GpiSetBitmap(hpsRB[Bitmap],hbmRB[Bitmap]);
                                        /* Select selected bitmap into client area */
if(!(GpiSetBitmap(hpsDT,hbmRB[Bitmap])))
    GEN_ERR(habDT,hwndFrame,hwndClient);
                                        /* Copy bitmap from memory to client area */
if(GpiBitBlt(hpsDT,                     /* Target presentation space handle */
    hpsRB[Bitmap],                      /* Source presentation space handle */
    4L,                                 /* Point count */
    (PPOINTL)D_Array,                   /* Point array */
    mode,                               /* Mixing function required (copy over target) */
    BBO_IGNORE)==GPI_ERROR)             /* Options for compression (ignore it) */
    GEN_ERR(habDT,hwndFrame,hwndClient);
}

