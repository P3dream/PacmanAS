/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Interface to the simulator.
|
| History: 17/mar/01 - JCM - created.
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                       Arlindo Oliveira, aml@inesc.pt
|                 	Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/


#include "defs_inter.h"
#ifdef _WIN32
#include <stdlib.h>
#include <stdio.h>
#endif


static int  Tcl_Initialization(Tcl_Interp *);
static void create_commands(Tcl_Interp *);


Tcl_Interp *g_interp;        /* ioOutput needs this... */
int lin, colum;

int main(int argc, char *argv[])
{
#ifdef _WIN32
  if(argc < 2) {
    fprintf(stderr,"Usage %s <tcl specification>\n",argv[0]);
    return 1;
  }
  if(fopen(argv[1],"r") == NULL) {

    char pathbuffer[_MAX_PATH];
    char envvar[] = "PATH";

    /* Search for file in PATH environment variable: */
    _searchenv( argv[1], envvar, pathbuffer );
    if( *pathbuffer == '\0' ) {
      fprintf(stderr,"Can't find file %s\n",argv[1]);
      return 1;
    }
    argv[1] = (char *)malloc(strlen(pathbuffer)+1);
    strcpy(argv[1],pathbuffer);
      

  }
#endif
    reset_uP();

    Tk_Main(argc, argv, Tcl_Initialization);
    return 0;			/* Needed only to prevent compiler warning. */
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_Initialization --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in inter->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

static int Tcl_Initialization(Tcl_Interp *interp)
{
    Tk_Window main;

    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (Tk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

    main = Tk_MainWindow(interp);
    if (main == NULL) {
	return TCL_ERROR;
    }


    /*
     * Call the init procedures for included packages.  Each call should
     * look like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module.
     */

    /*
     * Call Tcl_CreateCommand for application-specific commands, if
     * they weren't already created by the init procedures called above.
     */

    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */

    Tcl_SetVar(interp, "tcl_RcFileName", "~/.wishrc", TCL_GLOBAL_ONLY);
    create_commands(interp);

    g_interp = interp;    /* make interp global... */

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
static void create_commands(Tcl_Interp *interp)
{
    Tcl_CreateCommand(interp, "regContent", regContent, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "pcContent", pcContent, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "spContent", spContent, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "reContent", reContent, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "memContent", memContent, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "ureContent", ureContent, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "carContent", carContent, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "irContent", irContent, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "sbrContent", sbrContent, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "stepProc", stepProc, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "clockProc", clockProc, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "unassembleAddr",unassembleAddr,(ClientData*)NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "loadProgram", loadProgram, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "dumpMemory", dumpMemory, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "loadMemory", loadMemory, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "resetPC", resetPC, (ClientData *) NULL,
                      (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "setRegisterValue", setRegisterValue,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "setMemoryValue", setMemoryValue,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "interruptProc", interruptProc,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "ioInput", ioInput,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL); 

    Tcl_CreateCommand(interp, "loadControlRom", loadControlRom,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "loadMapRom", loadMapRom,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "uiContent", uiContent,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "ivadContent", ivadContent,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "intContent", intContent,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "mapROMContent", mapROMContent,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "controlROMContent", controlROMContent,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "setIVADValue", setIVADValue,
                      (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
}
