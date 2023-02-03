%{
/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Lexical analyser.
|
| History: 23/mar/01 - JCM - created.
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                 	Arlindo Oliveira, aml@inesc.pt
|                       Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/

#define YYSTYPE char *
#define YY_USE_PROTOS
#include <stdio.h>
#include <stdlib.h>
#ifdef PC
#include "y_tab.h"
#else
#include "y.tab.h"
#endif
static void count(void);
static int comment(void);
extern int resolving;
/*#define ECHO if (!resolving) printf("%s",yytext);*/
#ifdef DEBUG
#define ECHO printf("%s",yytext);
#else
#define ECHO
#endif
/*#define yywrap() 1*/

void count(), setlval();
int column = 0;
int line = 1;
char ylbuf[1024];
char *ylbufptr = ylbuf;
%}

B                       [0-1]
O                       [0-7]
D                       [0-9]
L                       [a-zA-Z_]
H                       [a-fA-F0-9]
BS                      (b|B)
OS                      (o|O)
DS                      (d|D)
HS                      (h|H)


%%

";".*			{ /*comment */ count(); }

"i0op"		{ count(); return(I0OP); }  /*to be implemented in the lab*/
"i1op"		{ count(); return(I1OP); }  /*to be implemented in the lab*/
"i2op"		{ count(); return(I2OP); }  /*to be implemented in the lab*/
"ictl"		{ count(); return(ICTL); }  /*to be implemented in the lab*/


"loop"		{ count(); return(LOOP); }  /*to be implemented in the lab*/
"rnd"		{ count(); return(RND); }  /*to be implemented in the lab*/
"mul10"		{ count(); return(MUL10); }  /*to be implemented in the lab*/
"sxt"		{ count(); return(SXT); }  /*to be implemented in the lab*/
"slm"		{ count(); return(SLM); }  /*to be implemented in the lab*/

"nop"			{ count(); return(NOP); }
"eni"			{ count(); return(ENI); }
"dsi"			{ count(); return(DSI); }
"stc"			{ count(); return(STC); }
"clc"			{ count(); return(CLC); }
"cmc"			{ count(); return(CMC); }

"neg"			{ count(); return(NEG); }
"inc"			{ count(); return(INC); }
"dec"			{ count(); return(DEC); }
"com"			{ count(); return(COM); }
"push"			{ count(); return(PUSH); }
"pop"			{ count(); return(POP); }
"shr"			{ count(); return(SHR); }
"shl"			{ count(); return(SHL); }
"shra"			{ count(); return(SHRA); }
"shla"			{ count(); return(SHLA); }
"ror"			{ count(); return(ROR); }
"rol"			{ count(); return(ROL); }
"rorc"			{ count(); return(RORC); }
"rolc"			{ count(); return(ROLC); }

"cmp"			{ count(); return(CMP); }
"add"                   { count(); return(ADD); }
"addc"                  { count(); return(ADDC); }
"sub"			{ count(); return(SUB); }
"subb"			{ count(); return(SUBB); }
"mul"			{ count(); return(MUL); }
"div"			{ count(); return(DIV); }
"test"			{ count(); return(TEST); }
"and"			{ count(); return(AND); }
"or"			{ count(); return(OR); }
"xor"			{ count(); return(XOR); }
"mov"			{ count(); return(MOV); }
"mvbh"			{ count(); return(MVBH); }
"mvbl"			{ count(); return(MVBL); }
"xch"			{ count(); return(XCH); }

"ret"			{ count(); return(RET); }
"retn"			{ count(); return(RETN); }
"rti"			{ count(); return(RTI); }
"int"			{ count(); return(INT); }
"jmp"			{ count(); return(JMP); }
"jmp."		        { count(); return(JMPCOND); }
"br"			{ count(); return(BR); }
"br."			{ count(); return(BRCOND); }
"call"			{ count(); return(CALL); }
"call."		        { count(); return(CALLCOND); }

"p"		        { count(); return(P); }
"np"		        { count(); return(NP); }
"i"		        { count(); return(I); }
"ni"		        { count(); return(NI); }
"e"		        { count(); return(E); }
"ne"		        { count(); return(NE); }
"z"		        { count(); return(Z); }
"nz"		        { count(); return(NZ); }
"c"		        { count(); return(C); }
"nc"		        { count(); return(NC); }
"n"		        { count(); return(N); }
"nn"		        { count(); return(NN); }
"o"		        { count(); return(O); }
"no"		        { count(); return(NO); }

"r"[0-7]                { count(); setlval(); return(REG); }

"pc"			{ count(); return(PC);}
"sp"			{ count(); return(SP);}

"orig"			{ count(); return(ORIG); }
"str"			{ count(); return(STR); }
"equ"			{ count(); return(EQU); }
"word"			{ count(); return(WORD); }
"tab"			{ count(); return(TAB); }

","			{ count(); return(','); }
":"			{ count(); return(':'); }
"$"			{ count(); return('$'); }
"*"			{ count(); return('*'); }
"/"			{ count(); return('/'); }
"%"			{ count(); return('%'); }
"("			{ count(); return('('); }
")"			{ count(); return(')'); }
"["			{ count(); return('['); }
"]"			{ count(); return(']'); }
"\'"."\'"               { count(); setlval(); return(CHAR_LITERAL); }
"\'"[^'\n]+"\'"         { count(); setlval(); return(TEXT); }
"\'"[^\']+\n            { return(ERROR); }
"M"                     { count(); return(M); }
"+"			{ count(); setlval(); return(PLUS); }
"-"			{ count(); setlval(); return(MINUS); }


{B}+{BS}                { count(); setlval(); return(BIN_CONST); }
{O}+{OS}                { count(); setlval(); return(OCT_CONST); }
{D}+{DS}                { count(); setlval(); return(DEC_CONST); }
{D}+                    { count(); setlval(); return(DEC_CONST); }
{H}+{HS}                { count(); setlval(); return(HEX_CONST); }
{L}({L}|{D})*           { count(); setlval(); return(IDENTIFIER); }

[ \t\v\f]               { count(); }
[\n]			{ *ylbufptr = '\0'; ylbufptr = ylbuf;
			  column = 0; line++;
 			  return (NL);  }



%%





void setlval()
{

    extern char *yylval;
    yylval = (char *) malloc( strlen(yytext)+1);
    if (yylval == NULL)  {
        printf("Error: out of memory (malloc)\n");
        exit(-1);
    }
    strcpy(yylval, yytext);
}

comment()
{
    char c, c1;

  loop:
    while (((c = input()) != '*') && (c != 0))
        putchar(c);

    if(((c1 = input()) != '/') && (c != 0)){
        unput(c1);
        goto loop;
    }

    if(c != 0)
        putchar(c1);
}


void count()
{
    int i;

/*printf("yytext: '%s'\t\tColumn: %d\n", yytext, column);*/

    for (i = 0; yytext[i] != '\0'; i++) {
        *ylbufptr = yytext[i];
        if (ylbufptr - ylbuf > 1023) printf("YLBUF overflow (%d).\n",ylbufptr-ylbuf);
        if (yytext[i] == '\n') {
            abort();
/*            *(ylbufptr) = '#';
            column = 0;
            line ++;*/
        }
        else if (yytext[i] == '\t') {
            column += 8 - (column % 8);
        }
        else {
            column++;
        }
        ylbufptr++;
    }

    ECHO;
}

int check_type()
{
/*
* pseudo code --- this is what it should check
*
*       if (yytext == type_name)
*               return(TYPE_NAME);
*
*       return(IDENTIFIER);
*/

/*
*       it actually will only return IDENTIFIER
*/
	return(IDENTIFIER);
}
