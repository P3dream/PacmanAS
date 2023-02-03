/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Definitions for the Assembler.
|
| History: 23/mar/01 - JCM - created.
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                 	Arlindo Oliveira, aml@inesc.pt
|                       Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/
#include "../simulador/common.h"

#define FALSE 0
#define TRUE 1


/* Defines for operand types on the uCISC */
#define REGISTER 0
#define REGISTER_INDIRECT 1
#define IMMEDIATE 2
#define DIRECT 3
#define INDEXED 4
#define RELATIVE 51
#define RELATIVE_PLUS_0 52
#define BASED 61
#define BASED_PLUS_0 62


/* Types of labels */
#define CONSTANT 0
#define VAR_ADDR 1
#define STR_ADDR 2
#define TAB_ADDR 3
#define LABEL_ADDR 4


#define	MAXLBL 64 /* MODIFICADO lpc */
#define DEFAULT_ENTRY_LABEL "Inicio"
#define DEF_TAB_VALUE 0

/* Hack to write 8 bits at a time so that we don't have a problem porting
   binaries between machines */
#define FWRITE(data, sizeofword, pfile) \
{\
     char d;\
     int ii;\
     unsigned long ddata = *(data);\
     for(ii = 0; ii < (sizeofword); ii++){\
         d = ddata & 0xff;\
         fwrite(&d, 1, 1, (pfile));\
         ddata >>= 8;\
     }\
     cur_addr ++;\
}\


struct lbl {
    char name[MAXLBL];
    unsigned short val;
    char type;
    struct lbl *next;
};
