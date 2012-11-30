/***********************************************************************\
 *                               Error.c                               *
 *                 Copyright (C) by Stangl Roman, 1992                 *
 * This Code may be freely distributed, provided the Copyright isn't   *
 * removed.                                                            *
 *                                                                     *
 * Requires: Error.c    The routing General_Error                      *
 *           Error.h    The include-file that defines the macros       *
 *                      GEN_ERR, DOS_ERR                               *
 *                                                                     *
 * Error is a general errorhandler for OS/2 2.0 PM programmer to easy  *
 * program development. The routine General_Error displays:            *
 *      ErrModule       The module containing the error                *
 *      ErrLine         The sourcecode line, that contains the error   *
 *      Error           The error returned by an OS/2 API              *
 * The routine requires the following parameters passed:               *
 *      HAB hab         The anchor block handle                        *
 *      HWND hwndFrame  The windowhandle of the frame window           *
 *      HWND hwndClient The windowhandle of the client window          *
 *      PSZ ErrModule   The pointer to the name of the module __FILE__ *
 *      LONG ErrLine    The pointer to the sourcecodeline __LINE__     *
 *                                                                     *
 * The routine Dos_Error displays:                                     *
 *      ErrModule       The module containing the error                *
 *      ErrLine         The sourcecode line, that contains the error   *
 *      Error           The error returned by an OS/2 API              *
 * The routine requires the following parameters passed:               *
 *      ReturnCode      The returncode of an os/2 API                  *
 *      HWND hwndFrame  The windowhandle of the frame window           *
 *      HWND hwndClient The windowhandle of the client window          *
 *      PSZ ErrModule   The pointer to the name of the module __FILE__ *
 *      LONG ErrLine    The pointer to the sourcecodeline __LINE__     *
 *                                                                     *
 * The routine User_Error displays:                                    *
 *      ErrModule       The module containing the error                *
 *      ErrLine         The sourcecode line, that contains the error   *
 *      Error           The error indicated by the user                *
 * The routine requires the following parameters passed:               *
 *      PSZ ErrorCode   The errorcode of the user                      *
 *      HWND hwndFrame  The windowhandle of the frame window           *
 *      HWND hwndClient The windowhandle of the client window          *
 *      PSZ ErrModule   The pointer to the name of the module __FILE__ *
 *      LONG ErrLine    The pointer to the sourcecodeline __LINE__     *
\***********************************************************************/

#include        "error.h"               /* Include headerfile */

void General_Error(HAB hab,HWND hwndFrame,HWND hwndClient,PSZ ErrModule,LONG ErrLine)
{
static PERRINFO         pErrInfoBlk;    /* Pointer to ERRINFO structure that is filled
                                           by WinGetErrorInfo */
static PSZ              pszOffset;      /* Pointer to the current error message returned
                                           by WinGetErrorInfo */
static unsigned char    ErrBuffer[512]; /* The whole error message that is displayed to
                                           the user via WinMessageBox */

sprintf(ErrBuffer,"Error found in Module: %s\nLinenumber in Sourcecode: %d\n"\
    "Error reported by OS/2: ",ErrModule,ErrLine);
                                        /* Get last error for the current thread. We loop
                                           until no more error is found, although errors
                                           arn't stacked (but things may change) */
while((pErrInfoBlk = WinGetErrorInfo(hab)) != (PERRINFO)NULL)
    {
    DosBeep(1000,200);                  /* Signal to user via speaker */
    DosBeep(300,200);
    DosBeep(1000,200);
                                        /* Find offset in array of message offsets */
    pszOffset = ((PSZ)pErrInfoBlk) + pErrInfoBlk->offaoffszMsg;
                                        /* Address error message in array of messages and 
                                           append error message to source code linenumber */
    strcat(ErrBuffer,(((PSZ)pErrInfoBlk) + *((PSHORT)pszOffset)));
    if((INT)hwndFrame && (INT)hwndClient)
    if((WinMessageBox(HWND_DESKTOP,     /* Parent window is DESKTOP */
        hwndFrame,                      /* Owner window is our frame */
        (PSZ)ErrBuffer,                 /* General_Error message */
                                        /* Title bar message */
        "General Error Message Information",
        GEN_ERR_MSGBOXID,               /* Message identifier */
                                        /* Buttons Abort, Retry, Ignore */
        MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION | MB_DEFBUTTON3))==MBID_ABORT)
            {                           /* If user selected Abort, then close application */
            WinPostMsg(hwndClient, WM_QUIT, (MPARAM)NULL, (MPARAM)NULL);
            }
    WinFreeErrorInfo(pErrInfoBlk);      /* Free resource segment */
    }
}

void Dos_Error(ULONG Error,HWND hwndFrame,HWND hwndClient,PSZ ErrModule,LONG ErrLine)
{
static unsigned char    MsgBuffer[512]; /* The whole error message that is displayed to
                                           the user via WinMessageBox */
static ULONG            Class;          /* Error class */
static ULONG            Action;         /* Error action */
static ULONG            Locus;          /* Error location */

static unsigned char    *Err_Class[]={"Out of ressource",
                                      "Temporary situation",
                                      "Authorization failed",
                                      "Internal error",
                                      "Device hardware failure",
                                      "System failure",
                                      "Probable application failure",
                                      "Item not located",
                                      "Bad format for call/data",
                                      "Resource or data locked",
                                      "Incorrect media, CRC check",
                                      "Action already taken or done",
                                      "Unclassified",
                                      "Cannot perform requested action",
                                      "Timeout",
                                      "Error in file \"Error.c\""};
static unsigned char    *Err_Action[]={"Retry immediately",
                                       "Delay and retry",
                                       "Bad user input - get new values",
                                       "Terminate in an orderly manner",
                                       "Terminate immediately",
                                       "Ignore error",
                                       "Retry after user intervention",
                                       "Error in file \"Error.c\""};
static unsigned char    *Err_Locus[]={"Unknown",
                                      "Random access device such as a disk",
                                      "Network",
                                      "Serial device",
                                      "Memory",
                                      "Error in file \"Error.c\""};

if(Error!=0)
{
    DosBeep(1000,200);                  /* Signal to user via speaker */
    DosBeep(300,200);
    DosBeep(1000,200);
    DosErrClass(Error,&Class,&Action,&Locus);
    sprintf(MsgBuffer,"Error found in Module: %s\nLinenumber in Sourcecode: %d\n"\
        "Error reported by OS/2: %d\nClass: %s\nAction: %s\n"\
        "Location: %s\n",ErrModule,ErrLine,Error,Err_Class[Class-1],\
        Err_Action[Action-1],Err_Locus[Locus-1]);
    if((INT)hwndFrame && (INT)hwndClient)
    if((WinMessageBox(HWND_DESKTOP,     /* Parent window is DESKTOP */
        hwndFrame,                      /* Owner window is our frame */
        (PSZ)MsgBuffer,                 /* DOS API error message */
                                        /* Title bar message */
        "OS/2 DosApi Error Message Information",
        GEN_ERR_MSGBOXID,               /* Message identifier */
                                        /* Buttons Abort, Retry, Ignore */
        MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION | MB_DEFBUTTON3))==MBID_ABORT)
            {                           /* If user selected Abort, then close application */
            WinPostMsg(hwndClient, WM_QUIT, (MPARAM)NULL, (MPARAM)NULL);
            }
}
}

void User_Error(PSZ Error,HWND hwndFrame,HWND hwndClient,PSZ ErrModule,LONG ErrLine)
{
static unsigned char    MsgBuffer[512]; /* The whole error message that is displayed to
                                           the user via WinMessageBox */

DosBeep(1000,200);                      /* Signal to user via speaker */
DosBeep(300,200);
DosBeep(1000,200);
sprintf(MsgBuffer,"Error found in Module: %s\nLinenumber in Sourcecode: %d\n"\
    "%s\n",ErrModule,ErrLine,Error);
if((INT)hwndFrame && (INT)hwndClient)
if((WinMessageBox(HWND_DESKTOP,         /* Parent window is DESKTOP */
    hwndFrame,                          /* Owner window is our frame */
    (PSZ)MsgBuffer,                     /* User indicated error message */
                                        /* Title bar message */
    "User indicated Error Message Information",
    GEN_ERR_MSGBOXID,                   /* Message identifier */
                                        /* Buttons Abort, Retry, Ignore */
    MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION | MB_DEFBUTTON3))==MBID_ABORT)
                                        /* If user selected Abort, then close application */
    WinPostMsg(hwndClient, WM_QUIT, (MPARAM)NULL, (MPARAM)NULL);
}
