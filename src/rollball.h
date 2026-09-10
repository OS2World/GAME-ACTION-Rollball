extern int      main(void);
extern int      Draw_Initialize(void);
extern void     Playground_Initialize(void);
extern void     Draw_Thread(ULONG ulThreadArg);

                                        /* Window and resource defines */
#define ID_WINDOW       256
#define ID_POINTER      257
#define BM_ABOUT        258
#define BM_RB           259
#define BM_LX           260
#define BM_RX           261
#define BM_HOLE         262
#define BM_BP           263
#define BM_GP           264
#define BM_MP           265
#define BM_VP           266
#define OS2LOGO         267

                                        /* Dialog identifiers */
#define ID_ABOUTDIALOG  270
#define ID_SCOREDIALOG  271

                                        /* Game menu (100-199) */
#define IDM_SUBMENU_GAME    1000
#define IDM_PLAY            101
#define IDM_PAUSE           102
#define IDM_QUIT            103
#define IDM_EXIT            104

                                        /* Options menu (200-299) */
#define IDM_SUBMENU_OPTS    200
#define IDM_BACKGRND        201
#define IDM_FRAME           202
#define IDM_SAVEONEXIT      203
#define IDM_SCORE           204

                                        /* Language submenu (300-399) */
#define IDM_SUBMENU_LANG    350
#define IDM_LANG_EN         301
#define IDM_LANG_ES         302
#define IDM_LANG_NL         303
#define IDM_LANG_DE         304
#define IDM_LANG_FR         305
#define IDM_LANG_IT         306

                                        /* Help menu (900-999) */
#define IDM_SUBMENU_HELP    900
#define IDM_ABOUT           999

                                        /* Playing field dimensions */
#define RB_X            44
#define RB_Y            32
#define RB_SIZE         24      /* display cell size (pixels) */
#define RB_BMPSIZE      15      /* native bitmap size in resource */

                                        /* Drawing thread messages */
#define DT_EXIT         WM_USER+1
#define DT_PAINT        WM_USER+2
#define DT_INITIALIZE   WM_USER+3
#define DT_LBUTTON      WM_USER+4
#define DT_RBUTTON      WM_USER+5
#define DT_IDLE         WM_USER+6

extern HAB      hab;
extern HWND     hwndFrame, hwndClient;
extern PSZ      pszErrMsg;
extern HMQ      hmqDT;
extern TID      tidDT;
extern BOOL     runRB;
extern ULONG    RB_Point[];
