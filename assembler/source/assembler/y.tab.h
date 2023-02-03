/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NOP = 258,
     ENI = 259,
     DSI = 260,
     CMC = 261,
     STC = 262,
     CLC = 263,
     RET = 264,
     RTI = 265,
     RETN = 266,
     INT = 267,
     PUSH = 268,
     POP = 269,
     INC = 270,
     DEC = 271,
     NEG = 272,
     COM = 273,
     SHR = 274,
     SHL = 275,
     SHRA = 276,
     SHLA = 277,
     ROR = 278,
     ROL = 279,
     RORC = 280,
     ROLC = 281,
     MOV = 282,
     MVBH = 283,
     MVBL = 284,
     XCH = 285,
     ADD = 286,
     ADDC = 287,
     SUB = 288,
     SUBB = 289,
     MUL = 290,
     DIV = 291,
     CMP = 292,
     AND = 293,
     OR = 294,
     XOR = 295,
     TEST = 296,
     JMP = 297,
     JMPCOND = 298,
     BR = 299,
     BRCOND = 300,
     CALL = 301,
     CALLCOND = 302,
     P = 303,
     NP = 304,
     I = 305,
     NI = 306,
     E = 307,
     NE = 308,
     Z = 309,
     NZ = 310,
     C = 311,
     NC = 312,
     N = 313,
     NN = 314,
     O = 315,
     NO = 316,
     MUL10 = 317,
     SXT = 318,
     SLM = 319,
     RND = 320,
     LOOP = 321,
     I0OP = 322,
     I1OP = 323,
     I2OP = 324,
     ICTL = 325,
     HEX_CONST = 326,
     DEC_CONST = 327,
     OCT_CONST = 328,
     BIN_CONST = 329,
     ORIG = 330,
     STR = 331,
     EQU = 332,
     WORD = 333,
     TAB = 334,
     PC = 335,
     SP = 336,
     M = 337,
     CHAR_LITERAL = 338,
     IDENTIFIER = 339,
     CTRLCOND = 340,
     TEXT = 341,
     PLUS = 342,
     MINUS = 343,
     NL = 344,
     REG = 345,
     ERROR = 346
   };
#endif
/* Tokens.  */
#define NOP 258
#define ENI 259
#define DSI 260
#define CMC 261
#define STC 262
#define CLC 263
#define RET 264
#define RTI 265
#define RETN 266
#define INT 267
#define PUSH 268
#define POP 269
#define INC 270
#define DEC 271
#define NEG 272
#define COM 273
#define SHR 274
#define SHL 275
#define SHRA 276
#define SHLA 277
#define ROR 278
#define ROL 279
#define RORC 280
#define ROLC 281
#define MOV 282
#define MVBH 283
#define MVBL 284
#define XCH 285
#define ADD 286
#define ADDC 287
#define SUB 288
#define SUBB 289
#define MUL 290
#define DIV 291
#define CMP 292
#define AND 293
#define OR 294
#define XOR 295
#define TEST 296
#define JMP 297
#define JMPCOND 298
#define BR 299
#define BRCOND 300
#define CALL 301
#define CALLCOND 302
#define P 303
#define NP 304
#define I 305
#define NI 306
#define E 307
#define NE 308
#define Z 309
#define NZ 310
#define C 311
#define NC 312
#define N 313
#define NN 314
#define O 315
#define NO 316
#define MUL10 317
#define SXT 318
#define SLM 319
#define RND 320
#define LOOP 321
#define I0OP 322
#define I1OP 323
#define I2OP 324
#define ICTL 325
#define HEX_CONST 326
#define DEC_CONST 327
#define OCT_CONST 328
#define BIN_CONST 329
#define ORIG 330
#define STR 331
#define EQU 332
#define WORD 333
#define TAB 334
#define PC 335
#define SP 336
#define M 337
#define CHAR_LITERAL 338
#define IDENTIFIER 339
#define CTRLCOND 340
#define TEXT 341
#define PLUS 342
#define MINUS 343
#define NL 344
#define REG 345
#define ERROR 346




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

