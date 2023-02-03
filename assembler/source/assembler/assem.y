%{
/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Syntax checker.
|
| History: 23/mar/01 - JCM - created.
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                 	Arlindo Oliveira, aml@inesc.pt
|                       Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/


#include <string.h>
#ifdef PC
#include <stdlib.h>
#endif

#include "assem.h"
#define YYDEBUG 1

extern void produce(int, int *, int *, unsigned char **);
extern void create_label(char *, short, int);
extern int get_label(char *, unsigned short *);
void hextodec(char *, char *);
void octtodec(char *, char *);
void bintodec(char *, char *);
int yyerror(char *);


extern char yytext[];
extern int cur_addr;

#define MAX_OPVALS 1000

int optypes[MAX_OPVALS];
unsigned char *opvals[MAX_OPVALS];
int op_cnt=0;
unsigned short value;


#define ADD_VAL(type,val)  optypes[op_cnt] = (type); \
                           opvals[op_cnt++] = (val) ;

%}


%token NOP ENI DSI CMC STC CLC RET RTI
%token RETN INT
%token PUSH POP INC DEC NEG COM
%token SHR SHL SHRA SHLA ROR ROL RORC ROLC 
%token MOV MVBH MVBL XCH ADD ADDC SUB SUBB MUL DIV CMP AND OR XOR TEST 
%token JMP JMPCOND BR BRCOND CALL CALLCOND
%token P NP I NI E NE Z NZ C NC N NN O NO
%token MUL10 SXT SLM RND LOOP
%token I0OP I1OP I2OP ICTL

%token HEX_CONST DEC_CONST OCT_CONST BIN_CONST
%token ORIG STR EQU WORD TAB

%token PC SP M CHAR_LITERAL IDENTIFIER CTRLCOND TEXT PLUS MINUS
%token NL
%token REG
%token ERROR


%start top
%%

top : code | code last_line ;

/*top : blank code | code  | blank code last_line | code last_line ;*/

last_line : label instruction | instruction | instrpseudo ;  /* accept last line without NL */

code : code inst |  inst ;

inst : label instruction NL | instruction NL  | instrpseudo NL | NL ;

/*blank : blank NL | NL ;*/

label : IDENTIFIER ':' {create_label((char*)$1, (short)cur_addr, LABEL_ADDR); };

instruction : instr0 | instr0s | instr1 | instr1s | instr2 | instrctrl ;



/*---------------------------------------------------------------------------*/

instr0 : zero_op_code  {produce ($1, &op_cnt, optypes, opvals); };

zero_op_code :
  NOP {$$ = NOP; }
| ENI {$$ = ENI; }
| DSI {$$ = DSI; }
| CMC {$$ = CMC; }
| STC {$$ = STC; }
| CLC {$$ = CLC; }
| RET {$$ = RET; }
| RTI {$$ = RTI; }
| I0OP  {$$ = I0OP; } ;

instr0s :
special_zero_op_code identifier
{ADD_VAL(IDENTIFIER,(char *)$2); produce($1,&op_cnt,optypes,opvals); } ;

special_zero_op_code:
  RETN {$$ = RETN; }
| INT  {$$ = INT; } ;


/*---------------------------------------------------------------------------*/

instr1 : one_op_code all_op {produce ($1, &op_cnt, optypes, opvals); };

one_op_code :
  PUSH  {$$ = PUSH; }
| POP   {$$ = POP; }
| INC   {$$ = INC; }
| DEC   {$$ = DEC; }
| NEG   {$$ = NEG; }
| COM   {$$ = COM; }
| LOOP  {$$ = LOOP; }
| RND   {$$ = RND; }
| MUL10 {$$ = MUL10; }
| SXT   {$$ = SXT; }
| I1OP  {$$ = I1OP; } ;

instr1s :
special_one_op_code non_const_op ',' identifier
{ADD_VAL(IDENTIFIER,(char *)$4); produce($1,&op_cnt,optypes,opvals); } ;

special_one_op_code:
  SHR  {$$ = SHR; }
| SHL  {$$ = SHL; }
| SHRA {$$ = SHRA; }
| SHLA {$$ = SHLA; }
| ROR  {$$ = ROR; }
| ROL  {$$ = ROL; }
| RORC {$$ = RORC; }
| ROLC {$$ = ROLC; };


/*---------------------------------------------------------------------------*/

instr2 :
two_op_code register_op ',' register_op {produce($1,&op_cnt,optypes,opvals);}|
two_op_code register_op ',' non_reg_op {produce($1,&op_cnt,optypes,opvals); } |
two_op_code op ',' register_op {produce($1,&op_cnt,optypes,opvals);}|
MOV register_op ',' register_op {produce(MOV,&op_cnt,optypes,opvals);}|
MOV register_op ',' non_reg_op {produce(MOV,&op_cnt,optypes,opvals); } |
MOV op ',' register_op {produce(MOV,&op_cnt,optypes,opvals);}|
MOV SP ',' register_op {produce(SP,&op_cnt,optypes,opvals);} |
MOV register_op ',' SP {produce(PC,&op_cnt,optypes,opvals);}; /* Terrible hack: use PC to indicate reverse */

two_op_code :
  MVBH {$$ = MVBH; }
| MVBL {$$ = MVBL; }
| XCH  {$$ = XCH;  } 
| ADD  {$$ = ADD;  }
| ADDC {$$ = ADDC; }
| SUB  {$$ = SUB;  }
| SUBB {$$ = SUBB; }
| MUL  {$$ = MUL;  }
| DIV  {$$ = DIV;  }
| CMP  {$$ = CMP;  }
| AND  {$$ = AND;  }
| OR   {$$ = OR;   }
| XOR  {$$ = XOR;  }
| TEST {$$ = TEST; }
| SLM  {$$ = SLM;  }
| I2OP  {$$ = I2OP; } ;


/*---------------------------------------------------------------------------*/

instrctrl :
  absjmpcall_op_code all_op
      {produce($1,&op_cnt,optypes,opvals);}
| condjmpcall_op_code ctrl_cond all_op
      {ADD_VAL(CTRLCOND,(char *)$2); produce($1,&op_cnt,optypes,opvals);}
| absbr_op_code constant_op
      {produce($1,&op_cnt,optypes,opvals);}
| condbr_op_code ctrl_cond constant_op 
      {ADD_VAL(CTRLCOND,(char *)$2); produce($1,&op_cnt,optypes,opvals);} ;

absjmpcall_op_code :
  JMP    {$$=JMP; }
| CALL   {$$=CALL; }
| ICTL  {$$ = ICTL; } ;

condjmpcall_op_code :
  JMPCOND    {$$=JMPCOND; }
| CALLCOND   {$$=CALLCOND; };

absbr_op_code :
  BR         {$$=BR; };

condbr_op_code :
  BRCOND     {$$=BRCOND; };

ctrl_cond :
  P   {$$=P; }
| NP  {$$=NP; }
| I   {$$=I; }
| NI  {$$=NI; }
| E   {$$=E; }
| NE  {$$=NE; }
| Z   {$$=Z; }
| NZ  {$$=NZ; }
| C   {$$=C; }
| NC  {$$=NC; }
| N   {$$=N; }
| NN  {$$=NN; }
| O   {$$=O; }
| NO  {$$=NO; };


/*---------------------------------------------------------------------------*/

instrpseudo : instrpseudo1 | instrpseudo2 | instrpseudo3 | instrpseudo4 ;

instrpseudo1 :
IDENTIFIER op_wordequ constant {ADD_VAL(IDENTIFIER,(char *)$1);
                                ADD_VAL(IMMEDIATE,(char *)$3);
                                produce($2,&op_cnt,optypes,opvals); };
op_wordequ : WORD {$$=WORD; } | EQU {$$=EQU;};


instrpseudo2 :
IDENTIFIER op_str string {ADD_VAL(IDENTIFIER,(char *)$1);
                          produce($2,&op_cnt,optypes,opvals); };
op_str : STR {$$=STR;};

instrpseudo3 :
IDENTIFIER op_tab identifier      {ADD_VAL(IDENTIFIER,(char *)$1);
                                   ADD_VAL(IMMEDIATE,(char *)$3);
                                   produce($2,&op_cnt,optypes,opvals); };
op_tab : TAB {$$=TAB;};

instrpseudo4 :
op_orig unsigned_number {ADD_VAL(IMMEDIATE,(char *)$2);
                         produce($1,&op_cnt,optypes,opvals);};
op_orig : ORIG {$$=ORIG;};



/*---------------------------------------------------------------------------*/

all_op:
op | constant_op | register_op;

non_reg_op:
op | constant_op;

non_const_op:
op | register_op;

register_op:
register
{ADD_VAL(REGISTER, (char *)$1);};

constant_op:
constant
{ADD_VAL(IMMEDIATE,(char *)$1); };

op :
M '[' register  ']'
{ADD_VAL(REGISTER_INDIRECT,(char *)$3); }

| M '[' unsigned_constant  ']'
{ADD_VAL(DIRECT,(char *)$3);}

| M '[' register plus_minus unsigned_constant  ']'
{ADD_VAL(INDEXED,(char *)$3);
 ADD_VAL(INDEXED,(char *)$4);
 ADD_VAL(INDEXED,(char *)$5);}

| M '[' PC  plus_minus unsigned_constant  ']'
{ADD_VAL(RELATIVE,(char *)$4);
 ADD_VAL(RELATIVE,(char *)$5);}

| M '[' PC  ']'
{ADD_VAL(RELATIVE_PLUS_0,"");}

| M '[' SP plus_minus unsigned_constant ']'
{ADD_VAL(BASED,(char *)$4);
 ADD_VAL(BASED,(char *)$5);}

| M '[' SP ']'
{ADD_VAL(BASED_PLUS_0,(char *)"");} ;


plus_minus: PLUS | MINUS ;

register : REG {$$=($1+1); };

string: string1
| string ',' string1 ;
/*{$$=$1;
mystrcat((char *) $$, (char *) $3);} ;
((char*)$$)[mystrlen((char*)$$)-1]='\0';
strncat((char*)$$,(char*)&(((char*)$3)[1]), mystrlen((char *)$3));} ; */

string1: 
  TEXT            {ADD_VAL(TEXT, (char *) $1);}
| CHAR_LITERAL    {ADD_VAL(TEXT, (char *) $1);}
| unsigned_number {ADD_VAL(IMMEDIATE, (char *) atoi((char *) $1));}
| IDENTIFIER      {ADD_VAL(IDENTIFIER,(char*) $1);};


constant: signed_number | IDENTIFIER;

unsigned_constant: identifier | CHAR_LITERAL ;

identifier: unsigned_number | IDENTIFIER;

signed_number:
  CHAR_LITERAL          {$$=$1; }
| unsigned_number       {$$=$1; }
| PLUS unsigned_number  {$$=$2; }
| MINUS unsigned_number {$$=$1; strcat((char*)$$,(char*)$2); } ;

unsigned_number:
  BIN_CONST {bintodec((char *) $$, (char*) $1); }
| OCT_CONST {octtodec((char *) $$, (char*) $1); }
| HEX_CONST {hextodec((char *) $$, (char*) $1); }
| DEC_CONST {$$=$1;} ;


/*---------------------------------------------------------------------------*/
%%

#include <stdio.h>

extern int column, line;

int yyerror(char *s)
{
    extern char ylbuf[], *ylbufptr;

    fflush(stdout);
    fprintf(stderr, "Error(s) encountered while processing:\n");
    if (ylbufptr != ylbuf) *ylbufptr = '\0';
    fprintf(stderr, "%s\n",ylbuf);
    fprintf(stderr, "\n%*s%s\n%s (line %d). Exiting.\n", column-1, "",
            "^ syntax error", s, line+1);
    exit(-1);
}
/* int yyerror(char *s) */
/* { */
/* #define SYNTAX_MSG "^ syntax error" */

/*     extern char ylbuf[], *ylbufptr; */
/*     static char synt[] = SYNTAX_MSG; */
/*     int offset; */

/*     offset = column + strlen(SYNTAX_MSG); */

/*     fflush(stdout); */
/*     fprintf(stderr, "Error(s) encountered while processing:\n"); */
/*     if (ylbufptr != ylbuf) *ylbufptr = '\0'; */
/*     fprintf(stderr, "%s\n",ylbuf); */
/*     fprintf(stderr, "\n%*s\n%*s (line %d)\n", offset, "  ^ syntax error",  */
/*             offset, s, line+1); */
/*     fprintf(stderr, "Exiting.\n"); */
/*     exit(-1); */
/* } */


/* Assumes valid hex characters */
void hextodec(char *resul, char *orig)
{
   int x = 0;

   /* hex string ends with h/H */
   while((*orig != 'h') && (*orig != 'H')){
      x = x << 4;
      *orig |= 32;                /* convert to lower case */
      if(*orig >= 'a')
          x |= *orig -'a' + 10;
      else
          x |= *orig - '0';
      orig++;
   }

   sprintf(resul, "%d", x);
}

/* Assumes valid oct characters */
void octtodec(char *resul, char *orig)
{
   int x = 0;

   /* oct string ends with o/O */
   while((*orig != 'o') && (*orig != 'O')){
      x = x << 3;
      x |= *orig - '0';
      orig++;
   }

   sprintf(resul, "%d", x);
}

/* Assumes valid bin characters */
void bintodec(char *resul, char *orig)
{
   int x = 0;

   /* bin string ends with b/B */
   while((*orig != 'b') && (*orig != 'B')){
      x = x << 1;
      x |= *orig - '0';
      orig++;
   }

   sprintf(resul, "%d", x);
}
