/***********************************************************************\
 *                             RollBall                                *
 *  OS/2 Presentation Manager game — roll a ball to earn points.      *
 *  Original: Stangl Roman, 1992.  OW port: Martin Iturbide, 2026.    *
\***********************************************************************/

#define INCL_WIN
#define INCL_WINSYS
#define INCL_WINPOINTERS
#define INCL_GPI
#define INCL_DEV
#define INCL_WINDIALOGS
#define INCL_DOSPROCESS
#define INCL_WINMENUS

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "error.h"
#include "rollball.h"
#include "lang.h"

#pragma off(unreferenced)
static const char bldlevel[] =
    "@#Stangl Roman:1.3#@##1## 10 Sep 2026 20:00:00      "
    "ARCAOS:::0::::@@RollBall Game for OS/2\r\n\x1a";
#pragma on(unreferenced)

/* ------------------------------------------------------------------ */
/* Language strings                                                     */
/* ------------------------------------------------------------------ */

int current_lang = LANG_EN;

const char *lang_strings[LANG_COUNT][STR_COUNT] = {
    /* EN */ {
        "~Game", "~New Game\tCtrl+N", "~Pause Game\tCtrl+P", "~Quit Game\tCtrl+Q", "E~xit\tCtrl+X",
        "~Options", "Score ~Info...", "~Language", "~Save settings on exit",
        "~Background Run\tCtrl+B", "~Frame Controls\tCtrl+F",
        "~Help", "~About...",
        "RollBall Score",
        "Thank you for playing RollBall!\nYour score is %d point(s)!\nHope you had a good time!"
    },
    /* ES */ {
        "~Juego", "~Nuevo Juego\tCtrl+N", "~Pausar Juego\tCtrl+P", "~Terminar Juego\tCtrl+Q", "~Salir\tCtrl+X",
        "~Opciones", "Puntuacion...", "~Idioma", "~Guardar al salir",
        "Fondo ~Activo\tCtrl+B", "~Marco\tCtrl+F",
        "A~yuda", "~About...",
        "Puntuacion RollBall",
        "Gracias por jugar RollBall!\nTu puntuacion es %d punto(s)!\nEspero que lo hayas disfrutado!"
    },
    /* NL */ {
        "~Spel", "~Nieuw Spel\tCtrl+N", "~Pauze Spel\tCtrl+P", "~Stop Spel\tCtrl+Q", "~Afsluiten\tCtrl+X",
        "~Opties", "Score ~Info...", "~Taal", "~Opslaan bij afsluiten",
        "~Achtergrond Actief\tCtrl+B", "~Frame Bediening\tCtrl+F",
        "~Help", "~About...",
        "RollBall Score",
        "Bedankt voor het spelen van RollBall!\nJe score is %d punt(en)!\nHoop dat je het leuk vond!"
    },
    /* DE */ {
        "~Spiel", "~Neues Spiel\tCtrl+N", "~Pause\tCtrl+P", "~Spiel beenden\tCtrl+Q", "~Beenden\tCtrl+X",
        "~Optionen", "Punkte ~Info...", "~Sprache", "~Einstellungen speichern",
        "~Hintergrundlauf\tCtrl+B", "~Rahmen\tCtrl+F",
        "~Hilfe", "~About...",
        "RollBall Punkte",
        "Danke fuer das Spielen von RollBall!\nDeine Punktzahl ist %d Punkt(e)!\nIch hoffe, du hattest Spass!"
    },
    /* FR */ {
        "~Jeu", "~Nouveau Jeu\tCtrl+N", "~Pause Jeu\tCtrl+P", "~Quitter Jeu\tCtrl+Q", "~Sortir\tCtrl+X",
        "~Options", "~Info Score...", "~Langue", "~Enregistrer a la sortie",
        "~Arriere-plan Actif\tCtrl+B", "~Cadre\tCtrl+F",
        "~Aide", "~About...",
        "Score RollBall",
        "Merci d'avoir joue a RollBall!\nVotre score est de %d point(s)!\nJ'espere que vous avez apprecie!"
    },
    /* IT */ {
        "~Gioco", "~Nuovo Gioco\tCtrl+N", "~Pausa Gioco\tCtrl+P", "~Ferma Gioco\tCtrl+Q", "~Esci\tCtrl+X",
        "~Opzioni", "~Info Punteggio...", "~Lingua", "~Salva all'uscita",
        "~Sfondo Attivo\tCtrl+B", "~Cornice\tCtrl+F",
        "~Aiuto", "~About...",
        "Punteggio RollBall",
        "Grazie per aver giocato a RollBall!\nIl tuo punteggio e %d punto/i!\nSpero che ti sia divertito!"
    }
};

/* ------------------------------------------------------------------ */
/* Globals                                                              */
/* ------------------------------------------------------------------ */

HAB     hab;
PSZ     pszErrMsg;
TID     tidDT;

int     AreaX;
int     AreaY;
int     AreaSizeX;
int     AreaSizeY;
BOOL    AreaActive = FALSE;

HWND    hwndFrame  = NULLHANDLE;
HWND    hwndClient = NULLHANDLE;

HPOINTER hptrCrossHair;

HDC     hdcMem;
HPS     hpsMem;
HBITMAP hbmMem;

static BOOL bBackgrndRun = FALSE;
static BOOL bFrameHidden = FALSE;
static BOOL bSaveOnExit  = FALSE;

static HWND hwndTitleBar = NULLHANDLE;
static HWND hwndSysMenu  = NULLHANDLE;
static HWND hwndMinMax   = NULLHANDLE;
static HWND hwndMenuBar  = NULLHANDLE;

/* ------------------------------------------------------------------ */
/* Settings persistence                                                 */
/* ------------------------------------------------------------------ */

#define SETTINGS_FILE "rollball.cfg"

typedef struct {
    int saveonexit;
    int current_lang;
} SETTINGS;

static void load_settings(void)
{
    FILE *f = fopen(SETTINGS_FILE, "rb");
    SETTINGS s;
    memset(&s, 0, sizeof(s));
    if (f) {
        fread(&s, sizeof(s), 1, f);
        fclose(f);
    }
    bSaveOnExit  = (s.saveonexit != 0);
    current_lang = s.current_lang;
    if (current_lang < 0 || current_lang >= LANG_COUNT) current_lang = LANG_EN;
}

static void save_settings(void)
{
    FILE *f = fopen(SETTINGS_FILE, "wb");
    SETTINGS s;
    if (!f) return;
    s.saveonexit  = bSaveOnExit ? 1 : 0;
    s.current_lang = current_lang;
    fwrite(&s, sizeof(s), 1, f);
    fclose(f);
}

/* ------------------------------------------------------------------ */
/* Language application                                                 */
/* ------------------------------------------------------------------ */

static void set_language(HWND hwndMenu, int lang)
{
    MENUITEM mi;
    HWND hwndSub;
    int i;

    current_lang = lang;

    /* Update Game submenu items */
    memset(&mi, 0, sizeof(mi));
    mi.iPosition   = MIT_NONE;
    mi.afStyle     = MIS_TEXT;
    mi.afAttribute = 0;
    mi.id          = IDM_SUBMENU_GAME;
    WinSendMsg(hwndMenu, MM_QUERYITEM,
               MPFROM2SHORT(IDM_SUBMENU_GAME, FALSE), MPFROMP(&mi));
    hwndSub = mi.hwndSubMenu;
    if (hwndSub) {
        WinSendMsg(hwndSub, MM_SETITEMTEXT,
                   MPFROMLONG(IDM_PLAY),  MPFROMP(tr(STR_MENU_PLAY)));
        WinSendMsg(hwndSub, MM_SETITEMTEXT,
                   MPFROMLONG(IDM_PAUSE), MPFROMP(tr(STR_MENU_PAUSE)));
        WinSendMsg(hwndSub, MM_SETITEMTEXT,
                   MPFROMLONG(IDM_QUIT),  MPFROMP(tr(STR_MENU_QUIT)));
        WinSendMsg(hwndSub, MM_SETITEMTEXT,
                   MPFROMLONG(IDM_EXIT),  MPFROMP(tr(STR_MENU_EXIT)));
    }
    /* Update top-level submenu titles */
    WinSendMsg(hwndMenu, MM_SETITEMTEXT,
               MPFROMLONG(IDM_SUBMENU_GAME), MPFROMP(tr(STR_MENU_GAME)));

    /* Update Options submenu items */
    memset(&mi, 0, sizeof(mi));
    WinSendMsg(hwndMenu, MM_QUERYITEM,
               MPFROM2SHORT(IDM_SUBMENU_OPTS, FALSE), MPFROMP(&mi));
    hwndSub = mi.hwndSubMenu;
    if (hwndSub) {
        WinSendMsg(hwndSub, MM_SETITEMTEXT,
                   MPFROMLONG(IDM_SCORE),      MPFROMP(tr(STR_MENU_SCORE)));
        WinSendMsg(hwndSub, MM_SETITEMTEXT,
                   MPFROMLONG(IDM_SUBMENU_LANG), MPFROMP(tr(STR_MENU_LANGUAGE)));
        WinSendMsg(hwndSub, MM_SETITEMTEXT,
                   MPFROMLONG(IDM_SAVEONEXIT), MPFROMP(tr(STR_MENU_SAVEONEXIT)));
        WinSendMsg(hwndSub, MM_SETITEMTEXT,
                   MPFROMLONG(IDM_BACKGRND),   MPFROMP(tr(STR_MENU_BACKGRND)));
        WinSendMsg(hwndSub, MM_SETITEMTEXT,
                   MPFROMLONG(IDM_FRAME),      MPFROMP(tr(STR_MENU_FRAME)));
    }
    WinSendMsg(hwndMenu, MM_SETITEMTEXT,
               MPFROMLONG(IDM_SUBMENU_OPTS), MPFROMP(tr(STR_MENU_OPTIONS)));

    /* Update Help submenu */
    memset(&mi, 0, sizeof(mi));
    WinSendMsg(hwndMenu, MM_QUERYITEM,
               MPFROM2SHORT(IDM_SUBMENU_HELP, FALSE), MPFROMP(&mi));
    hwndSub = mi.hwndSubMenu;
    if (hwndSub) {
        WinSendMsg(hwndSub, MM_SETITEMTEXT,
                   MPFROMLONG(IDM_ABOUT), MPFROMP(tr(STR_MENU_ABOUT)));
    }
    WinSendMsg(hwndMenu, MM_SETITEMTEXT,
               MPFROMLONG(IDM_SUBMENU_HELP), MPFROMP(tr(STR_MENU_HELP)));

    /* Check the selected language item */
    for (i = IDM_LANG_EN; i <= IDM_LANG_IT; i++)
        WinCheckMenuItem(hwndMenu, i, (i - IDM_LANG_EN == lang));
}

/* ------------------------------------------------------------------ */
/* Dialog procedures                                                    */
/* ------------------------------------------------------------------ */

MRESULT EXPENTRY BallWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY AboutDialog(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ScoreDialog(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* ------------------------------------------------------------------ */
/* main()                                                               */
/* ------------------------------------------------------------------ */

int main(void)
{
    DEVOPENSTRUC DcData = {NULL,"DISPLAY",NULL,NULL,NULL,NULL,NULL,NULL,NULL};
    SIZEL        sizellogo = {81, 81};
    HMQ          hmq;
    QMSG         qmsg;
    ULONG        flCreate = FCF_SYSMENU | FCF_MINBUTTON | FCF_ICON |
                            FCF_MENU | FCF_DLGBORDER |
                            FCF_TASKLIST | FCF_TITLEBAR | FCF_ACCELTABLE;
    LONG         cxScreen, cyScreen, winW, winH, x, y;

    DosSetPriority(0, PRTYC_TIMECRITICAL, 0, 0);

    if ((hab = WinInitialize(0)) == 0L)
        GEN_ERR(hab, hwndFrame, hwndClient);

    /* Draw_Initialize() is now called inside Draw_Thread() after habDT is
       ready, so all hpsRB[] handles share the same HAB as hpsDT. */

    if ((hdcMem = DevOpenDC(hab, OD_MEMORY, (PSZ)"*", 8L,
                            (PDEVOPENDATA)&DcData, (HDC)NULL)) == 0L)
        GEN_ERR(hab, hwndFrame, hwndClient);

    if ((hpsMem = GpiCreatePS(hab, hdcMem, &sizellogo,
                              GPIA_ASSOC | PU_PELS)) == 0L)
        GEN_ERR(hab, hwndFrame, hwndClient);

    if ((hbmMem = GpiLoadBitmap(hpsMem, 0L, OS2LOGO, 0L, 0L)) == 0L)
        GEN_ERR(hab, hwndFrame, hwndClient);

    if ((hmq = WinCreateMsgQueue(hab, 0)) == 0L)
        GEN_ERR(hab, hwndFrame, hwndClient);

    load_settings();

    if (!WinRegisterClass(hab, (PSZ)"BallWindow",
                          (PFNWP)BallWindowProc, CS_SIZEREDRAW, 0))
        GEN_ERR(hab, hwndFrame, hwndClient);

    if ((hwndFrame = WinCreateStdWindow(
             HWND_DESKTOP, 0, &flCreate,
             "BallWindow", "", 0,
             (HMODULE)0L, ID_WINDOW,
             &hwndClient)) == 0L)
        GEN_ERR(hab, hwndFrame, hwndClient);

    /* Cache frame control handles (must be after WinCreateStdWindow) */
    hwndTitleBar = WinWindowFromID(hwndFrame, FID_TITLEBAR);
    hwndSysMenu  = WinWindowFromID(hwndFrame, FID_SYSMENU);
    hwndMinMax   = WinWindowFromID(hwndFrame, FID_MINMAX);
    hwndMenuBar  = WinWindowFromID(hwndFrame, FID_MENU);

    /* Apply persisted language */
    set_language(hwndMenuBar, current_lang);
    WinCheckMenuItem(hwndMenuBar, IDM_SAVEONEXIT, bSaveOnExit);
    WinCheckMenuItem(hwndMenuBar, IDM_BACKGRND,   bBackgrndRun);

    /* Size and center window */
    cxScreen = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
    cyScreen = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
    AreaSizeX = RB_SIZE * (RB_X - 2) + 8;
    AreaSizeY = RB_SIZE * (RB_Y - 2) + 6
              + WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR)
              + WinQuerySysValue(HWND_DESKTOP, SV_CYMENU);
    winW = (AreaSizeX < cxScreen) ? AreaSizeX : cxScreen;
    winH = (AreaSizeY < cyScreen) ? AreaSizeY : cyScreen;
    x    = (cxScreen - winW) / 2;
    y    = (cyScreen - winH) / 2;

    WinSetWindowText(hwndFrame, "RollBall - OS/2 PM Game");
    if (!WinSetWindowPos(hwndFrame, HWND_TOP, (SHORT)x, (SHORT)y,
                         (SHORT)winW, (SHORT)winH,
                         SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW))
        GEN_ERR(hab, hwndFrame, hwndClient);

    while (WinGetMsg(hab, &qmsg, 0UL, 0UL, 0UL))
        WinDispatchMsg(hab, &qmsg);

    WinDestroyWindow(hwndFrame);
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Window procedure                                                     */
/* ------------------------------------------------------------------ */

MRESULT EXPENTRY BallWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
    case WM_CREATE:
        {
        static ULONG  ThreadArg   = 0;
        static ULONG  ThreadFlags = 0;
        static ULONG  StackSize   = 8192;
        static APIRET rc;

        AreaActive    = TRUE;
        hptrCrossHair = WinLoadPointer(HWND_DESKTOP, 0L, ID_POINTER);
        if ((rc = DosCreateThread(&tidDT, (PFNTHREAD)Draw_Thread,
                                  ThreadArg, ThreadFlags, StackSize)) != 0)
            DOS_ERR(rc, hwndFrame, hwndClient);
        break;
        }

    case WM_USER:
        {
        HPS    hps;
        POINTL bma[4];

        bma[0].x = (RB_SIZE*(RB_X-2)/2 - 40);
        bma[0].y = (RB_SIZE*(RB_Y-2)/2 - 40);
        bma[1].x = bma[0].x + 81;
        bma[1].y = bma[0].y + 81;
        bma[2].x = bma[2].y = 0;
        bma[3].x = bma[3].y = 80;
        hps = WinGetPS(hwnd);
        GpiSetBitmap(hpsMem, hbmMem);
        GpiSetBitmap(hps, hbmMem);
        GpiBitBlt(hps, hpsMem, 4L, (PPOINTL)bma, ROP_SRCCOPY, BBO_IGNORE);
        WinReleasePS(hps);
        break;
        }

    case WM_BUTTON1DOWN:
        if (!WinSetFocus(HWND_DESKTOP, hwnd))
            GEN_ERR(hab, hwndFrame, hwndClient);
        if (runRB == TRUE)
            WinPostQueueMsg(hmqDT, DT_LBUTTON,
                            MPFROMLONG(SHORT1FROMMP(mp1)),
                            MPFROMLONG(SHORT2FROMMP(mp1)));
        break;

    case WM_BUTTON2DOWN:
        if (!WinSetFocus(HWND_DESKTOP, hwnd))
            GEN_ERR(hab, hwndFrame, hwndClient);
        if (runRB == TRUE)
            WinPostQueueMsg(hmqDT, DT_RBUTTON,
                            MPFROMLONG(SHORT1FROMMP(mp1)),
                            MPFROMLONG(SHORT2FROMMP(mp1)));
        break;

    case WM_SETFOCUS:
        {
        static BOOL bFocusPaused = FALSE;
        HWND hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);

        if (SHORT1FROMMP(mp2)) {   /* gaining focus */
            WinSetPointer(HWND_DESKTOP, hptrCrossHair);
            if (bFocusPaused && !bBackgrndRun) {
                runRB = TRUE;
                if (hwndMenu) WinCheckMenuItem(hwndMenu, IDM_PAUSE, FALSE);
            }
            bFocusPaused = FALSE;
        } else {                   /* losing focus */
            if (!bBackgrndRun && runRB) {
                runRB = FALSE;
                bFocusPaused = TRUE;
                if (hwndMenu) WinCheckMenuItem(hwndMenu, IDM_PAUSE, TRUE);
            }
        }
        return (MRESULT)WinDefWindowProc(hwnd, msg, mp1, mp2);
        }

    case WM_MOUSEMOVE:
        if (AreaActive)
            WinSetPointer(HWND_DESKTOP, hptrCrossHair);
        break;

    case WM_MINMAXFRAME:
        {
        static BOOL runRBsave = FALSE;
        if (((PSWP)mp1)->fl & SWP_MINIMIZE) {
            WinSetWindowText(hwndFrame, "RollBall");
            if ((runRBsave = runRB) == TRUE) runRB = FALSE;
        } else {
            WinSetWindowText(hwndFrame, "RollBall - OS/2 PM Game");
            runRB = runRBsave;
        }
        return (MRESULT)WinDefWindowProc(hwnd, msg, mp1, mp2);
        }

    case WM_COMMAND:
        {
        HWND  hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);
        USHORT cmd     = SHORT1FROMMP(mp1);

        switch (cmd)
        {
        case IDM_PLAY:
            {
            if (runRB) break;
            if (hwndMenu) {
                WinSendMsg(hwndMenu, MM_SETITEMATTR,
                           MPFROM2SHORT(IDM_PLAY, TRUE),
                           MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
                WinSendMsg(hwndMenu, MM_SETITEMATTR,
                           MPFROM2SHORT(IDM_PAUSE, TRUE),
                           MPFROM2SHORT(MIA_DISABLED, 0));
                WinSendMsg(hwndMenu, MM_SETITEMATTR,
                           MPFROM2SHORT(IDM_QUIT, TRUE),
                           MPFROM2SHORT(MIA_DISABLED, 0));
            }
            /* Post to drawing thread — all drawing must happen on the thread
               that owns hpsDT and hpsRB[]; cross-thread PS use is not allowed. */
            WinPostQueueMsg(hmqDT, DT_INITIALIZE, 0UL, 0UL);
            break;
            }

        case IDM_QUIT:
            {
            BOOL runRBsave = runRB;
            char MsgBuffer[512];
            if (!runRB) break;
            runRB = FALSE;
            if (hwndMenu) {
                WinSendMsg(hwndMenu, MM_SETITEMATTR,
                           MPFROM2SHORT(IDM_QUIT, TRUE),
                           MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
                WinSendMsg(hwndMenu, MM_SETITEMATTR,
                           MPFROM2SHORT(IDM_PAUSE, TRUE),
                           MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
                WinSendMsg(hwndMenu, MM_SETITEMATTR,
                           MPFROM2SHORT(IDM_PLAY, TRUE),
                           MPFROM2SHORT(MIA_DISABLED, 0));
                WinCheckMenuItem(hwndMenu, IDM_PAUSE, FALSE);
            }
            sprintf(MsgBuffer, tr(STR_GAMEOVER_MSG), RB_Point[0]);
            WinMessageBox(HWND_DESKTOP, hwndFrame,
                          (PSZ)MsgBuffer, (PSZ)tr(STR_GAMEOVER_TITLE),
                          0, MB_INFORMATION | MB_OK);
            (void)runRBsave;
            break;
            }

        case IDM_PAUSE:
            /* IDM_PAUSE is only enabled while a game is running */
            runRB = !runRB;
            WinCheckMenuItem(hwndMenuBar, IDM_PAUSE, !runRB);
            break;

        case IDM_EXIT:
            WinPostMsg(hwnd, WM_CLOSE, (MPARAM)0, (MPARAM)0);
            break;

        case IDM_SCORE:
            {
            BOOL runRBsave = runRB;
            runRB = FALSE;
            WinDlgBox(HWND_DESKTOP, hwndFrame, ScoreDialog,
                      (HMODULE)0, ID_SCOREDIALOG, NULL);
            runRB = runRBsave;
            break;
            }

        case IDM_ABOUT:
            {
            BOOL runRBsave = runRB;
            runRB = FALSE;
            WinDlgBox(HWND_DESKTOP, hwndFrame, AboutDialog,
                      (HMODULE)0, ID_ABOUTDIALOG, NULL);
            runRB = runRBsave;
            break;
            }

        case IDM_SAVEONEXIT:
            bSaveOnExit = !bSaveOnExit;
            if (hwndMenu)
                WinCheckMenuItem(hwndMenu, IDM_SAVEONEXIT, bSaveOnExit);
            break;

        case IDM_BACKGRND:
            bBackgrndRun = !bBackgrndRun;
            if (hwndMenu)
                WinCheckMenuItem(hwndMenu, IDM_BACKGRND, bBackgrndRun);
            break;

        case IDM_FRAME:
            {
            HWND hwndTarget;
            bFrameHidden = !bFrameHidden;
            hwndTarget   = bFrameHidden ? HWND_OBJECT : hwndFrame;
            WinSetParent(hwndTitleBar, hwndTarget, FALSE);
            WinSetParent(hwndSysMenu,  hwndTarget, FALSE);
            WinSetParent(hwndMinMax,   hwndTarget, FALSE);
            WinSetParent(hwndMenuBar,  hwndTarget, FALSE);
            WinSendMsg(hwndFrame, WM_UPDATEFRAME,
                       (MPARAM)(FCF_TITLEBAR | FCF_SYSMENU |
                                FCF_MINMAX | FCF_MENU), NULL);
            WinInvalidateRect(hwndFrame, NULL, TRUE);
            WinUpdateWindow(hwndFrame);
            WinCheckMenuItem(hwndMenuBar, IDM_FRAME, bFrameHidden);
            break;
            }

        case IDM_LANG_EN: case IDM_LANG_ES: case IDM_LANG_NL:
        case IDM_LANG_DE: case IDM_LANG_FR: case IDM_LANG_IT:
            set_language(hwndMenuBar, cmd - IDM_LANG_EN);
            break;

        default:
            return (MRESULT)WinDefWindowProc(hwnd, msg, mp1, mp2);
        }
        break;
        }

    case WM_PAINT:
        {
        static BOOL logoinit = TRUE;
        HPS hps;
        hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
        WinPostQueueMsg(hmqDT, DT_PAINT, 0UL, 0UL);
        WinEndPaint(hps);
        if (logoinit) {
            logoinit = FALSE;
            DosSleep(50);
            WinPostMsg(hwnd, WM_USER, NULL, NULL);
        }
        break;
        }

    case WM_CLOSE:
        if (bSaveOnExit) save_settings();
        WinPostQueueMsg(hmqDT, DT_EXIT, 0UL, 0UL);
        DosWaitThread(&tidDT, DCWW_WAIT);
        WinPostMsg(hwnd, WM_QUIT, NULL, NULL);
        break;

    default:
        return (MRESULT)WinDefWindowProc(hwnd, msg, mp1, mp2);
    }
    return (MRESULT)FALSE;
}

/* ------------------------------------------------------------------ */
/* Dialog procedures                                                    */
/* ------------------------------------------------------------------ */

MRESULT EXPENTRY AboutDialog(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    if (msg == WM_COMMAND) { WinDismissDlg(hwnd, TRUE); return (MRESULT)0; }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ScoreDialog(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    if (msg == WM_COMMAND) { WinDismissDlg(hwnd, TRUE); return (MRESULT)0; }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
