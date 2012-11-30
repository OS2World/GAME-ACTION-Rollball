/***********************************************************************\
 *                               Error.h                               *
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

#define         INCL_WIN
#define         INCL_DOSMISC

#include        <os2.h>
#include        <stdio.h>
#include        <string.h>

#define         GEN_ERR_MSGBOXID 61234  /* This number should'nt occur in any other program */
#define         GEN_ERR(x,y,z) General_Error(x,y,z,__FILE__,__LINE__)
#define         DOS_ERR(x,y,z) Dos_Error(x,y,z,__FILE__,__LINE__)
#define         USR_ERR(x) User_Error(x,hwndFrame,hwndClient,__FILE__,__LINE__)

extern void General_Error(HAB hab,HWND hwndFrame,HWND hwndClient,PSZ ErrModule,LONG ErrLine);
extern void Dos_Error(ULONG Error,HWND hwndFrame,HWND hwndClient,PSZ ErrModule,LONG ErrLine);
extern void User_Error(PSZ Error,HWND hwndFrame,HWND hwndClient,PSZ ErrModule,LONG ErrLine);

