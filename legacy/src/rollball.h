extern int      main(void);
extern int      Draw_Initialize(void);
extern void     Playground_Initialize(void);
extern void     Draw_Thread(ULONG ulThreadArg);

					/* Window defines */
#define		ID_WINDOW	256
#define		ID_POINTER	257
#define		BM_ABOUT	258	/* About dialogbox bitmap */
#define         BM_RB		259	/* RollBall bitmap */
#define         BM_LX           260     /* \ bitmap */
#define         BM_RX           261     /* / bitmap */
#define         BM_HOLE         262     /* Hole bitmap */
#define         BM_BP           263     /* Blue point bitmap */
#define         BM_GP           264     /* Green point bitmap */
#define         BM_MP           265     /* Magenta point bitmap */
#define         BM_VP           266     /* Violett point bitmap */
#define         OS2LOGO         267     /* Intro logo bitmap */

#define		ID_ABOUTDIALOG	270	/* Dialog box identifiers */
#define		ID_SCOREDIALOG	271

#define		MSGBOXID	300
					/* Menu OPTIONS defines */
#define		ID_OPTIONS	400
#define		ID_ABOUT	401
#define		ID_SCORE	402
#define         ID_STARTTHREAD  403
#define         ID_STOPTHREAD   404
#define         ID_PAUSETHREAD  405
#define		ID_EXITPROGRAM	406

#define         RB_X            42      /* The size of the playing ground */
#define         RB_Y            27
#define         RB_SIZE         15      /* A field is 15x15 pels sized */

                                        /* DrawThread messages */
#define         DT_EXIT         WM_USER+1
#define         DT_PAINT        WM_USER+2
#define         DT_INITIALIZE   WM_USER+3
#define         DT_LBUTTON      WM_USER+4
#define         DT_RBUTTON      WM_USER+5
#define         DT_IDLE         WM_USER+6

extern HAB      hab;
extern HWND     hwndFrame,hwndClient;
extern PSZ      pszErrMsg;
extern HMQ      hmqDT;
extern TID      tidDT;
extern BOOL     runRB;
extern ULONG    RB_Point[];
