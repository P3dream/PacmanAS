/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Definitions for the interface to the simulator.
|
| History: 17/mar/01 - JCM - created.
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                       Arlindo Oliveira, aml@inesc.pt
|                 	Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/

#include <tcl.h>
#include <tk.h>
#include "common.h"


#define MEM_WORDS_PER_LINE 8

int regContent(ClientData, Tcl_Interp *, int, const char *[]);       /* commands.c */
int pcContent(ClientData, Tcl_Interp *, int, const char *[]);        /* commands.c */
int spContent(ClientData, Tcl_Interp *, int, const char *[]);        /* commands.c */
int reContent(ClientData, Tcl_Interp *, int, const char *[]);        /* commands.c */
int memContent(ClientData, Tcl_Interp *, int, const char *[]);       /* commands.c */
int carContent(ClientData, Tcl_Interp *, int, const char *[]);       /* commands.c */
int irContent(ClientData, Tcl_Interp *, int, const char *[]);        /* commands.c */
int sbrContent(ClientData, Tcl_Interp *, int, const char *[]);       /* commands.c */
int ureContent(ClientData, Tcl_Interp *, int, const char *[]);       /* commands.c */
int stepProc(ClientData, Tcl_Interp *, int, const char *[]);         /* commands.c */
int clockProc(ClientData, Tcl_Interp *, int, const char *[]);        /* commands.c */
int unassembleAddr(ClientData, Tcl_Interp *, int, const char *[]);   /* commands.c */
int loadProgram(ClientData, Tcl_Interp *, int, const char *[]);      /* commands.c */
int dumpMemory(ClientData, Tcl_Interp *, int, const char *[]);       /* commands.c */
int loadMemory(ClientData, Tcl_Interp *, int, const char *[]);       /* commands.c */
int loadControlRom(ClientData, Tcl_Interp *, int, const char *[]);   /* commands.c */
int loadMapRom(ClientData, Tcl_Interp *, int, const char *[]);       /* commands.c */
int resetPC(ClientData, Tcl_Interp *, int, const char *[]);          /* commands.c */
int setRegisterValue(ClientData, Tcl_Interp *, int, const char *[]); /* commands.c */
int setMemoryValue(ClientData, Tcl_Interp *, int, const char *[]);   /* commands.c */
int interruptProc(ClientData, Tcl_Interp *, int, const char *[]);    /* commands.c */
int ioInput(ClientData, Tcl_Interp *, int, const char *[]);          /* commands.c */
int uiContent(ClientData, Tcl_Interp *, int, const char *[]);        /* commands.c */
int ivadContent(ClientData, Tcl_Interp *, int, const char *[]);      /* commands.c */
int intContent(ClientData, Tcl_Interp *, int, const char *[]);       /* commands.c */
int mapROMContent(ClientData, Tcl_Interp *, int, const char *[]);    /* commands.c */
int controlROMContent(ClientData, Tcl_Interp *, int, const char *[]);/* commands.c */
int setIVADValue(ClientData, Tcl_Interp *, int, const char *[]);     /* commands.c */

int reg_content(int, char *);                     /* datapath.c */
int set_register(int, word);                      /* datapath.c */
void set_RE(mem_addr);                            /* datapath.c */

int execute1clock_cycle();                        /* control.c */
void pc_content(char *);                          /* control.c */
void sp_content(char *);                          /* control.c */
ctrlROM_addr car_content();                       /* control.c */
void ir_content(char *);                          /* control.c */
void sbr_content(char *);                         /* control.c */
void reset_uP();                                  /* control.c */
void ui_content(char *);                          /* control.c */
void int_content(char *);                         /* control.c */
void interrupt(ivad_index);                       /* control.c */

void newIOinput(char);                            /* mem.c */
int  content_main_memory(int, int, char *);       /* mem.c */
int  read_main_memory(mem_addr, word *);          /* mem.c */
void write_main_memory(mem_addr, word);           /* mem.c */


void code_unassemble();                           /* unassembler.c */
char *get_instruction(mem_addr);                  /* unassembler.c */

void write_control_ROM(ctrlROM_addr, dword);      /* microcode.c */

void         write_ROMA(word, ctrlROM_addr);      /* mapROM.c */
void         write_ROMB(word, ctrlROM_addr);      /* mapROM.c */
ctrlROM_addr read_ROMA(word);                     /* mapROM.c */
ctrlROM_addr read_ROMB(word);                     /* mapROM.c */
