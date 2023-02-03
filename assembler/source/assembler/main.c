/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Syntax checker.
|
| History: 23/mar/01 - JCM - created.
|           2/jan/03 - JCM - always write the contents of entire memory
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                 	Arlindo Oliveira, aml@inesc.pt
|                       Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/


#include <stdio.h>
#include "assem.h"

#ifdef _WIN32
static char *MOD_DATE = __DATE__;
#else
static char *MOD_DATE =
#include "mod_date.h"
;
#endif

#ifdef _WIN32
#define READ_BIN "rb"
#define WRITE_BIN "wb"
#else
#define READ_BIN "r"
#define WRITE_BIN "w"
#endif

#include "y.tab.h"

/* GLOBALS */
FILE *objf;
int pass;
int cur_addr;
char fn[64];
FILE *lis;
word memory[MAIN_MEM_SIZE];
char mem_used[MAIN_MEM_SIZE];
/* end globals */

/* yacc/lex flags and variables */
extern FILE *yyin;
extern int yydebug;
/* extern int yy_init; */
extern int line;

extern int get_label(char *, unsigned short *);
extern void dump_labels(FILE *);


main(int argc, char **argv)
{
    mem_addr addr,
	     start_addr,
	     size;
    int mark = P3AS_MARK;
    int i;
    char lfn[64],
         ofn[64];

    /* Initialization */
    yydebug = 0;
    line = 0;
    /* end initialization */

    fprintf(stdout, "%s, Version 1.3, last modified %s\n", argv[0], MOD_DATE);

    if(argc != 2){
        fprintf(stderr, "USAGE: %s input_file.as\n", argv[0]);
        return 1;
    }
    i = strlen(argv[1]);
    if(strcmp(&argv[1][i-3], ".as") != 0){
        fprintf(stderr, "Source file must have a .as extension. Bye.\n");
        return 1;
    }
    strcpy(fn, argv[1]);
    fn[i-3] = '\0';             /* remove extension from file name */
    yyin = fopen(argv[1], "r" );
    if(yyin == NIL(FILE)){
        fprintf(stderr, "Error opening input file %s\n", argv[1]);
        return 1;
    }

#ifdef DEBUG
    printf("Begining pass 1\n");
#endif
    pass = 1;
    cur_addr = 0;
    yyparse();
    rewind(yyin);


#ifdef DEBUG
    printf("Begining pass 2\n");
#endif

    for(i = 0; i < MAIN_MEM_SIZE; i++)
	mem_used[i] = 0;     /* Indicate all memory positions unused */

    pass = 2;
    line = 0; /* MODIFICADO lpc */
    cur_addr = 0;
    yyparse();
    fclose(yyin);

    /* Create object file, mask first before object code */
    strcpy(ofn, fn);
    strcat(ofn, ".exe");
    objf = fopen(ofn, WRITE_BIN);
    if(objf == NIL(FILE)){
        fprintf(stderr, "Error: Cannot create object file output.  Bye.\n");
        exit(-1);
    }
    FWRITE(&mark, sizeof(int), objf);

    for(addr = 0; addr < IO_ADDR; addr++){
	if(!mem_used[addr])
	    continue;        /* Search for start of next block of code */
	start_addr = addr;
	for(size = 1; start_addr + size < IO_ADDR; size++)
	    if(!mem_used[start_addr + size])
		break;       /* Search for end of current block of code */
	FWRITE(&size, sizeof(mem_addr), objf);       /* Write size of block */
	FWRITE(&start_addr, sizeof(mem_addr), objf); /* Write start address of block */
	for( ; addr < start_addr + size; addr++)
	    FWRITE(&memory[addr], sizeof(word), objf);  /* Write code for block */
/* printf("New Block! Start: %d\tSize: %d\n", start_addr, size); */

    }
    size = 0;
    FWRITE(&size, sizeof(word), objf);  /* size=0 indicates no more code */
    
    fprintf(stdout, "Assembling completed with success, object file: %s\n",ofn);

    strcpy(lfn, fn); strcat(lfn, ".lis");
    lis = fopen(lfn, "w");
    if(lis == NIL(FILE)){
        fprintf(stderr, "Error creating references file: %s\n", lfn);
        return 1;
    }

    dump_refs(lis);
    fclose(lis);

    fprintf(stdout, "References file: %s\n",lfn);

    return 0;
}

