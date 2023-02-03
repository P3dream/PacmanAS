/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2007
|
| Generates the binary code.
|
| History: 23/mar/01 - JCM - created.
|           2/nov/02 - JCM - new instruction format.
|           2/jan/03 - JCM - new ORIG format
|          18/fev/11 - JCM - swapped P/NP and E/NE codes, to conform with docs
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                 	Arlindo Oliveira, aml@inesc.pt
|                       Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/

#include "assem.h"
#include <stdio.h>
#include <string.h>
#include "y.tab.h"

#define INTERNAL_ERROR(msg) fprintf(stderr,"ASSEMBLER INTERNAL ERROR: %s\n",msg), exit(-1)


extern FILE *objf;
extern int pass;
extern int cur_addr;
extern word memory[];
extern char mem_used[];
extern int line; /* MODIFICADO lpc */

extern char *yyerror( char *);

struct lbl *lbl_base = NIL(struct lbl);
char tmp_buf[1000];

void create_label(unsigned char *, short, int);
int get_label(unsigned char *, unsigned short *);
word generate_arg_0op(unsigned char *);
word generate_modo_1op(int *, unsigned char **, int);
word generate_modo_shftrot(int *, unsigned char **, int, char *);
word generateM_S_2op(int *, unsigned char **, int);
void write_W_1op(int, unsigned char **);
void write_W_2op(int *, unsigned char **);
word generate_br_offset(unsigned char *);
word generate_condition(unsigned char **, int);
word translate_signed_constant(unsigned char *);
word translate_constant(char *, unsigned char *);
void memory_write(word);


/*-----------------------------------------------------------------------*/
void produce(int op, int *pop_cnt, int *optypes, unsigned char **opvals)
{
    int i, n,
        addr,
        optype,
        op_cnt = *pop_cnt;
    word data;
    char *text;

#ifdef DEBUG
    fprintf(stdout,"\nGot instruction of type %d with %d pars : ",op,op_cnt);
    for(i=0; i <  op_cnt; i++) {
	if((optypes[i] == CTRLCOND) || (optypes[i] == IMMEDIATE))
	    fprintf(stdout,"%d %d ; ",optypes[i],opvals[i]);
	else
	    fprintf(stdout,"%d %s ; ",optypes[i],opvals[i]);
    }
    fprintf(stdout,"\n");
#endif

    if(pass == 1){
        switch(op){
          case ORIG:
            addr = atoi(opvals[0]);
            if((optypes[0] != IMMEDIATE) || (addr < 0) || (addr > MAIN_MEM_SIZE))
                INTERNAL_ERROR("in ORIG statement.");
            cur_addr = (mem_addr) addr;
	    if(op_cnt != 1)
		INTERNAL_ERROR("in ORIG statement.");
            break;
          case EQU:
            if((op_cnt != 2) || (optypes[0] != IDENTIFIER) || (optypes[1] != IMMEDIATE))
                INTERNAL_ERROR("in EQU statement.");
            data = translate_signed_constant(opvals[1]);
            create_label(opvals[0], (mem_addr) data, CONSTANT);
            break;
          case WORD:
            if((op_cnt != 2) || (optypes[0] != IDENTIFIER) || (optypes[1] != IMMEDIATE))
                INTERNAL_ERROR("in WORD statement.");
            create_label(opvals[0], cur_addr, VAR_ADDR);
            cur_addr++;
            break;
          case STR:
            if(optypes[op_cnt-1] != IDENTIFIER)
                INTERNAL_ERROR("in STR statement.");
            create_label(opvals[op_cnt-1], cur_addr, STR_ADDR);
            for(i = 0; i < op_cnt - 1; i++)
                if(optypes[i] == IMMEDIATE)
                    cur_addr++;
                else
                    if(optypes[i] == TEXT)
                        cur_addr += mystrlen(opvals[i]) - 2;  /* The 2 quotes */
		    else
		        if(optypes[i] == IDENTIFIER)
                            cur_addr++;
			else
			    INTERNAL_ERROR("in STR statement.");
            break;
          case TAB:
            if((op_cnt != 2) || (optypes[0] != IDENTIFIER) || (optypes[1] != IMMEDIATE))
                INTERNAL_ERROR("in TAB statement.");
            create_label(opvals[0], cur_addr, TAB_ADDR);
            n = translate_signed_constant(opvals[1]);
            if((n < 1) || (n > 65535))
                yyerror("invalid number of memory positions.");
            cur_addr += n;
            break;

          case NOP:
          case ENI:
          case DSI:
          case CMC:
          case STC:
          case CLC:
          case RET:
          case RTI:
          case I0OP:
	    if(op_cnt != 0)
	        INTERNAL_ERROR("in zero-operand instruction.");
            cur_addr++;
            break;

          case RETN:
          case INT:
	    if(op_cnt != 1)
	        INTERNAL_ERROR("in zero-operand instruction.");
	    if(optypes[0] != IDENTIFIER)
	        INTERNAL_ERROR("in zero-operand instruction.");
            cur_addr++;
            break;

          case PUSH:
          case POP:
          case INC:
          case DEC:
          case NEG:
          case COM:
          case LOOP:
          case RND:
          case MUL10:
          case SXT:
          case I1OP:
            switch(optypes[0]){
              case REGISTER:
              case REGISTER_INDIRECT:
              case RELATIVE_PLUS_0:
              case BASED_PLUS_0:
                if(op_cnt != 1)
                    INTERNAL_ERROR("in one-operand instruction.");
                cur_addr++;
                break;
              case IMMEDIATE:
              case DIRECT:
                if(op_cnt != 1)
                    INTERNAL_ERROR("in one-operand instruction.");
                cur_addr += 2;
                break;
              case RELATIVE:
              case BASED:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in one-operand instruction.");
                cur_addr += 2;
                break;
              case INDEXED:
                if(op_cnt != 3)
                    INTERNAL_ERROR("in one-operand instruction.");
                cur_addr += 2;
                break;
              default:
                INTERNAL_ERROR("in one-operand instruction.");
            }
            break;

          case SHR:
          case SHL:
          case SHRA:
          case SHLA:
          case ROR:
          case ROL:
          case RORC:
          case ROLC:
            switch(optypes[0]){
              case REGISTER:
              case REGISTER_INDIRECT:
              case RELATIVE_PLUS_0:
              case BASED_PLUS_0:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in shift/rotate instruction.");
                cur_addr++;
                break;
              case IMMEDIATE:
              case DIRECT:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in shift/rotate instruction.");
                cur_addr += 2;
                break;
              case RELATIVE:
              case BASED:
                if(op_cnt != 3)
                    INTERNAL_ERROR("in shift/rotate instruction.");
                cur_addr += 2;
                break;
              case INDEXED:
                if(op_cnt != 4)
                    INTERNAL_ERROR("in shift/rotate instruction.");
                cur_addr += 2;
                break;
              default:
                INTERNAL_ERROR("in shift/rotate instruction.");
            }
            break;

          case CMP:
          case ADD:
          case ADDC:
          case SUB:
          case SUBB:
          case TEST:
          case AND:
          case OR:
          case XOR:
          case MOV:
          case MVBH:
          case MVBL:
          case SLM:
          case I2OP:
            if(optypes[0] == REGISTER)
                optype = optypes[1];
            else
                optype = optypes[0];
            switch(optype){
              case REGISTER:
              case REGISTER_INDIRECT:
              case RELATIVE_PLUS_0:
              case BASED_PLUS_0:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in two-operand instruction.");
                cur_addr++;
                break;
              case IMMEDIATE:
              case DIRECT:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in two-operand instruction.");
                cur_addr += 2;
                break;
              case RELATIVE:
              case BASED:
                if(op_cnt != 3)
                    INTERNAL_ERROR("in two-operand instruction.");
                cur_addr += 2;
                break;
              case INDEXED:
                if(op_cnt != 4)
                    INTERNAL_ERROR("in two-operand instruction.");
                cur_addr += 2;
                break;
              default:
                INTERNAL_ERROR("in two-operand instruction.");
            }
            break;

          case XCH:
          case MUL:
          case DIV:
            if(optypes[0] == REGISTER)
                optype = optypes[1];
            else
                optype = optypes[0];
            switch(optype){
              case REGISTER:
              case REGISTER_INDIRECT:
              case RELATIVE_PLUS_0:
              case BASED_PLUS_0:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in two-operand instruction.");
                cur_addr++;
                break;
              case IMMEDIATE:
		  yyerror("Instruction cannot have an immediate operand since both operands are used\nto store the result");
              case DIRECT:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in two-operand instruction.");
                cur_addr += 2;
                break;
              case RELATIVE:
              case BASED:
                if(op_cnt != 3)
                    INTERNAL_ERROR("in two-operand instruction.");
                cur_addr += 2;
                break;
              case INDEXED:
                if(op_cnt != 4)
                    INTERNAL_ERROR("in two-operand instruction.");
                cur_addr += 2;
                break;
              default:
                INTERNAL_ERROR("in two-operand instruction.");
            }
            break;

	  case SP:         /* Hack for special case MOV SP, reg */
	  case PC:         /* Hack for special case MOV reg, SP */
	    if((op_cnt != 1) || (optypes[0] != REGISTER))
		INTERNAL_ERROR("in 'MOV' instruction with SP.");
	    cur_addr += 1;
	    break;

          case JMP:
          case CALL:
          case ICTL:
            switch(optypes[0]){
              case REGISTER:
              case REGISTER_INDIRECT:
              case RELATIVE_PLUS_0:
              case BASED_PLUS_0:
                if(op_cnt != 1)
                    INTERNAL_ERROR("in branch instruction.");
                cur_addr++;
                break;
              case IMMEDIATE:
              case DIRECT:
                if(op_cnt != 1)
                    INTERNAL_ERROR("in branch instruction.");
                cur_addr += 2;
                break;
              case RELATIVE:
              case BASED:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in branch instruction.");
                cur_addr += 2;
                break;
              case INDEXED:
                if(op_cnt != 3)
                    INTERNAL_ERROR("in branch instruction.");
                cur_addr += 2;
                break;
              default:
                INTERNAL_ERROR("in branch instruction.");
            }
            break;

          case JMPCOND:
          case CALLCOND:
	    if(optypes[op_cnt-1] != CTRLCOND)
		INTERNAL_ERROR("in conditional branch instruction.");
            switch(optypes[0]){
              case REGISTER:
              case REGISTER_INDIRECT:
              case RELATIVE_PLUS_0:
              case BASED_PLUS_0:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in conditional branch instruction.");
                cur_addr++;
                break;
              case IMMEDIATE:
              case DIRECT:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in conditional branch instruction.");
                cur_addr += 2;
                break;
              case RELATIVE:
              case BASED:
                if(op_cnt != 3)
                    INTERNAL_ERROR("in conditional branch instruction.");
                cur_addr += 2;
                break;
              case INDEXED:
                if(op_cnt != 4)
                    INTERNAL_ERROR("in conditional branch instruction.");
                cur_addr += 2;
                break;
              default:
                INTERNAL_ERROR("in conditional branch instruction.");
            }
            break;

          case BR:
            switch(optypes[0]){
              case IMMEDIATE:
                if(op_cnt != 1)
                    INTERNAL_ERROR("in branch instruction.");
                cur_addr++;
                break;
              default:
                INTERNAL_ERROR("in branch instruction.");
            }
            break;

          case BRCOND:
	    if(optypes[op_cnt-1] != CTRLCOND)
		INTERNAL_ERROR("in conditional branch instruction.");
            switch(optypes[0]){
              case IMMEDIATE:
                if(op_cnt != 2)
                    INTERNAL_ERROR("in conditional branch instruction.");
                cur_addr++;
                break;
              default:
                INTERNAL_ERROR("in conditional branch instruction.");
            }
            break;

          default:
            INTERNAL_ERROR("unknown instruction code.");
        }

        if(cur_addr >= MAIN_MEM_SIZE)
            yyerror("program larger than the size of main memory.");
    }
    else    /* pass == 2 */
        switch(op){
          case ORIG:
            cur_addr = (mem_addr) atoi(opvals[0]);
	    break;
          case EQU:
	      /* Already taken care of */
            break;
          case WORD:
            memory_write(translate_signed_constant(opvals[1]));
            break;
          case STR:
            for(n = 0; n < op_cnt - 1; n++)
                if(optypes[n] == IMMEDIATE)
		    memory_write((word) opvals[n]);
		else
		    if(optypes[n] == TEXT)
		        for(i = 1; i < mystrlen(opvals[n]) - 1; i++)
			    memory_write((word) opvals[n][i]);
		    else{   /* IDENTIFIER */
		        if(!get_label(opvals[n], &data)){
                            strcpy(tmp_buf, opvals[n]);
                            yyerror(strcat(tmp_buf, ": undefined identifier"));
                        }
			memory_write(data);
		    }
            break;
          case TAB:
            n = translate_signed_constant(opvals[1]);
            data = DEF_TAB_VALUE;
            for(i = 0; i < n; i++)
		memory_write(data);
            break;

          case NOP:
            data = 0x0000;
	    memory_write(data);
            break;
          case ENI:
            data = 0x0400;
	    memory_write(data);
            break;
          case DSI:
            data = 0x0800;
	    memory_write(data);
            break;
          case STC:
            data = 0x0c00;
	    memory_write(data);
            break;
          case CLC:
            data = 0x1000;
	    memory_write(data);
            break;
          case CMC:
            data = 0x1400;
	    memory_write(data);
            break;
          case RET:
            data = 0x1800;
	    memory_write(data);
            break;
          case RTI:
            data = 0x1c00;
	    memory_write(data);
            break;
          case I0OP:     /* mesmo que MUL10: para implementar! */
            data = 0x3c00;
	    memory_write(data);
            break;

          case INT:
            data = generate_arg_0op(opvals[0]);
	    if(data > 255)
	        yyerror("value must be between 0 and 255");
            data |= 0x2000;
	    memory_write(data);
            break;
          case RETN:
            data = generate_arg_0op(opvals[0]);
	    if(data > 1023)
	        yyerror("value must be between 0 and 1023");
            data |= 0x2400;
	    memory_write(data);
            break;

          case NEG:
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x4000;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case INC:
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x4400;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case DEC:
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x4800;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case COM:
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x4c00;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case PUSH:
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x5000;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case POP:
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x5400;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
	case MUL10:   /* mesmo que SXT: para implementar! */
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x5800;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
	case SXT:     /* mesmo que MUL10: para implementar! */
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x5800;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case RND:     /* mesmo que MUL10: para implementar! */
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x5c00;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case LOOP:     /* mesmo que MUL10: para implementar! */
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x5c00;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case I1OP:     /* mesmo que MUL10: para implementar! */
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0x5c00;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;

          case SHR:
            data = generate_modo_shftrot(optypes, opvals, op, opvals[op_cnt-1]);
            data |= 0x6000;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case SHL:
            data = generate_modo_shftrot(optypes, opvals, op, opvals[op_cnt-1]);
            data |= 0x6400;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case SHRA:
            data = generate_modo_shftrot(optypes, opvals, op, opvals[op_cnt-1]);
            data |= 0x6800;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case SHLA:
            data = generate_modo_shftrot(optypes, opvals, op, opvals[op_cnt-1]);
            data |= 0x6c00;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case ROR:
            data = generate_modo_shftrot(optypes, opvals, op, opvals[op_cnt-1]);
            data |= 0x7000;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case ROL:
            data = generate_modo_shftrot(optypes, opvals, op, opvals[op_cnt-1]);
            data |= 0x7400;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case RORC:
            data = generate_modo_shftrot(optypes, opvals, op, opvals[op_cnt-1]);
            data |= 0x7800;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case ROLC:
            data = generate_modo_shftrot(optypes, opvals, op, opvals[op_cnt-1]);
            data |= 0x7c00;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;

          case CMP:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0x8000;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case ADD:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0x8400;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case ADDC:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0x8800;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case SUB:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0x8c00;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case SUBB:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0x9000;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case MUL:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0x9400;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case DIV:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0x9800;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case TEST:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0x9c00;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case AND:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0xa000;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case OR:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0xa400;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case XOR:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0xa800;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case MOV:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0xac00;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case SP:                      /* Hack for special case MOV SP, reg */
            data = 0xac0e | (atoi(opvals[0]) << 6);
	    memory_write(data);
            break;
          case PC:                      /* Hack for special case MOV reg, SP */
            data = 0xae0e | (atoi(opvals[0]) << 6);
	    memory_write(data);
	    break;
          case MVBH:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0xb000;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case MVBL:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0xb400;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case XCH:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0xb800;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case SLM:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0xbc00;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;
          case I2OP:
            data = generateM_S_2op(optypes, opvals, op_cnt);
            data |= 0xbc00;
	    memory_write(data);
            write_W_2op(optypes, opvals);
            break;

          case JMP:
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0xc000;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case JMPCOND:
            data = generate_modo_1op(optypes, opvals, op);
            data |= generate_condition(opvals, op_cnt);
            data |= 0xc400;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case CALL:
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0xc800;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case CALLCOND:
            data = generate_modo_1op(optypes, opvals, op);
            data |= generate_condition(opvals, op_cnt);
            data |= 0xcc00;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;
          case ICTL:
            data = generate_modo_1op(optypes, opvals, op);
            data |= 0xfc00;
	    memory_write(data);
            write_W_1op(optypes[0], opvals);
            break;

          case BR:
            data = generate_br_offset(opvals[0]);
            data |= 0xe000;
	    memory_write(data);
            break;
          case BRCOND:
            data = generate_br_offset(opvals[0]);
            data |= generate_condition(opvals, op_cnt);
            data |= 0xe400;
	    memory_write(data);
            break;

          default:
            INTERNAL_ERROR("unknown instruction code.");
        }

    *pop_cnt = 0;    /* Reset operand count for next instruction */
}


/*-----------------------------------------------------------------------*/
word generate_arg_0op(unsigned char *val)
{
    word data;

    data = translate_constant("+", val);

    return data;
}


/*-----------------------------------------------------------------------*/
word generate_modo_1op(int *type, unsigned char **val, int op)
{
    word data = 0;
    int ival;

    /* Register index is always the first value in opvals, except IMMEDIATE */
    ival = atoi(val[0]);

    switch(type[0]){
      case REGISTER:                    /* MODO = 0, REG = val */
          data = ival;
          break;
      case REGISTER_INDIRECT:           /* MODO = 1, REG = val */
          data = 0x0010 | ival;
          break;
      case IMMEDIATE:                   /* MODO = 2, REG = N/A */
	  switch(op){
            case NEG:
            case INC:
            case DEC:
            case COM:
            case POP:
            case MUL10:
            case SXT:
                yyerror("instruction cannot be used with IMMEDIATE mode");
	  }
          data = 0x0020;
          break;
      case DIRECT:                      /* MODO = 3, REG = R0 */
          data = 0x0030;
          break;
      case INDEXED:                     /* MODO = 3, REG = val */
          data = 0x0030 | ival;
          break;
      case RELATIVE:                    /* MODO = 3, REG = R15 = PC */
          data = 0x003f;
          break;
      case RELATIVE_PLUS_0:             /* MODO = 1, REG = R15 = PC */
          data = 0x001f;
          break;
      case BASED:                       /* MODO = 3, REG = R14 = SP */
          data = 0x003e;
          break;
      case BASED_PLUS_0:                /* MODO = 1, REG = R14 = SP */
          data = 0x001e;
          break;
      default:
        INTERNAL_ERROR("in 'generate_modo_1op'");
    }

    return data;
}


/*-----------------------------------------------------------------------*/
void write_W_1op(int type, unsigned char **vals)
{
    word data;

    switch(type){
      case REGISTER:
      case REGISTER_INDIRECT:
      case RELATIVE_PLUS_0:
      case BASED_PLUS_0:
        return;
      case IMMEDIATE:
      case DIRECT:
        data = translate_signed_constant(vals[0]);
        break;
      case RELATIVE:
      case BASED:
        data = translate_constant(vals[0], vals[1]);
        break;
      case INDEXED:
        data = translate_constant(vals[1], vals[2]);
        break;
      default:
        INTERNAL_ERROR("in 'write_W_1op'");
    }

    memory_write(data);
}


/*-----------------------------------------------------------------------*/
word generate_modo_shftrot(int *type, unsigned char **val, int op,char *samount)
{
    word data;
    int amount;

    data = generate_modo_1op(type, val, op);

    amount = translate_constant("+", samount);
    if((amount < 1) || (amount > 15))
        yyerror("invalid shift/rotate amount");

    data |= (amount << 6);

    return data;
}


/*-----------------------------------------------------------------------*/
word generateM_S_2op(int *types, unsigned char **val, int op_cnt)
{
    word data = 0;
    int ival0,
        ival1;

    if(types[0] == REGISTER){     /* destination is register */
        ival0 = atoi(val[0]);
        ival1 = atoi(val[1]);
        switch(types[1]){
          case REGISTER:                 /* MODO=0, S=1, REG1=val1, REG2=val0 */
            data = 0x0200 | ival1 | (ival0 << 6);
            break;
          case REGISTER_INDIRECT:        /* MODO=1, S=1, REG1=val1, REG2=val0 */
            data = 0x0210 | ival1 | (ival0 << 6);
            break;
          case IMMEDIATE:                /* MODO=2, S=1, REG1=N/A,  REG2=val0 */
            data = 0x0220 | (ival0 << 6);
            break;
          case DIRECT:                   /* MODO=3, S=1, REG1=R0,   REG2=val0 */
            data = 0x0230 | (ival0 << 6);
            break;
          case INDEXED:                  /* MODO=3, S=1, REG1=val1, REG2=val0 */
            data = 0x0230 | ival1 | (ival0 << 6);
            break;
          case RELATIVE:                 /* MODO=3, S=1, REG1=R15,  REG2=val0 */
            data = 0x023f | (ival0 << 6);
            break;
          case RELATIVE_PLUS_0:          /* MODO=1, S=1, REG1=R15,  REG2=val0 */
            data = 0x021f | (ival0 << 6);
            break;
          case BASED:                    /* MODO=3, S=1, REG1=R14,  REG2=val0 */
            data = 0x023e | (ival0 << 6);
            break;
          case BASED_PLUS_0:             /* MODO=1, S=1, REG1=R14,  REG2=val0 */
            data = 0x021e | (ival0 << 6);
            break;
          default:
            INTERNAL_ERROR("in 'generateM_S_2op'");
        }
    }
    else{                       /* source is register */
        ival0 = atoi(val[0]);
        ival1 = atoi(val[op_cnt-1]);
        if(types[op_cnt-1] != REGISTER)
            INTERNAL_ERROR("two operands, none of which is a register");
        switch(types[0]){
          case REGISTER_INDIRECT:        /* MODO=1, S=0, REG1=val0, REG2=val1 */
            data = 0x0010 | ival0 | (ival1 << 6);
            break;
          case IMMEDIATE:                /* Invalid */
            INTERNAL_ERROR("two-operand instruction, destination is constant");
          case DIRECT:                   /* MODO=3, S=0, REG1=R0,   REG2=val1 */
            data = 0x0030 | (ival1 << 6);
            break;
          case INDEXED:                  /* MODO=3, S=0, REG1=val0, REG2=val1 */
            data = 0x0030 | ival0 | (ival1 << 6);
            break;
          case RELATIVE:                 /* MODO=3, S=0, REG1=R15,  REG2=val1 */
            data = 0x003f | (ival1 << 6);
            break;
          case RELATIVE_PLUS_0:          /* MODO=1, S=0, REG1=R15,  REG2=val1 */
            data = 0x001f | (ival1 << 6);
            break;
          case BASED:                    /* MODO=3, S=0, REG1=R14,  REG2=val1 */
            data = 0x003e | (ival1 << 6);
            break;
          case BASED_PLUS_0:             /* MODO=1, S=0, REG1=R14,  REG2=val1 */
            data = 0x001e | (ival1 << 6);
            break;
          default:
            INTERNAL_ERROR("in 'generateM_S_2op'");
        }
    }

    return data;
}


/*-----------------------------------------------------------------------*/
void write_W_2op(int *types, unsigned char **vals)
{
    word data;

    if(types[0] == REGISTER)
        switch(types[1]){
          case REGISTER:
          case REGISTER_INDIRECT:
	  case RELATIVE_PLUS_0:
          case BASED_PLUS_0:
           return;
          case IMMEDIATE:
          case DIRECT:
            data = translate_signed_constant(vals[1]);
            break;
          case RELATIVE:
          case BASED:
            data = translate_constant(vals[1], vals[2]);
            break;
          case INDEXED:
            data = translate_constant(vals[2], vals[3]);
            break;
          default:
            INTERNAL_ERROR("in 'write_W_2op'");
        }
    else
        switch(types[0]){
          case REGISTER_INDIRECT:
          case RELATIVE_PLUS_0:
          case BASED_PLUS_0:
            return;
          case DIRECT:
            data = translate_signed_constant(vals[0]);
            break;
          case RELATIVE:
          case BASED:
            data = translate_constant(vals[0], vals[1]);
            break;
          case INDEXED:
            data = translate_constant(vals[1], vals[2]);
            break;
          default:
            INTERNAL_ERROR("in 'write_W_2op'");
        }

    memory_write(data);
}


/*-----------------------------------------------------------------------*/
word generate_condition(unsigned char **val, int op_cnt)
{
    /* Condition is always the last value in the stack */
    int cond = (int) val[op_cnt-1];

    switch(cond){
      case Z:
	return (0 << 6);
      case NZ:
	return (1 << 6);
      case C:
	return (2 << 6);
      case NC:
	return (3 << 6);
      case N:
	return (4 << 6);
      case NN:
	return (5 << 6);
      case O:
	return (6 << 6);
      case NO:
	return (7 << 6);
      case P:
	return (8 << 6);
      case NP:
	return (9 << 6);
      case I:
	return (10 << 6);
      case NI:
	return (11 << 6);
      case E:
        return (12 << 6);
      case NE:
	return (13 << 6);
      default:
        INTERNAL_ERROR("in 'generate_condition'");
    }
}


/*-----------------------------------------------------------------------*/
word generate_br_offset(unsigned char *val)
{
    int offset;

    if(val[0] == '+')
        INTERNAL_ERROR("'+' should not appear in signed constant.");

    if((val[0] >= '0') && (val[0] <= '9')){
        offset = translate_constant("+", val);
	if(offset > 31)
	    yyerror("target too far for branch instruction");
    }
    else if(val[0] == '-'){
        offset = translate_constant("+", &(val[1]));
	if(offset > 32)
	    yyerror("target too far for branch instruction");
	offset = -offset;
    }
    else{                          /* Relative to next address */
        offset = translate_constant("+", val) - (cur_addr + 1);
	if((offset < -32) || (offset > 31))
	    yyerror("target too far for branch instruction");
    }

    return (word)(offset & 0x003f);
}


/*-----------------------------------------------------------------------*/
word translate_signed_constant(unsigned char *val)
{
    word ival;

    if(val[0] == '+')
        INTERNAL_ERROR("'+' should not appear in signed constant.");

    if(val[0] == '-')
        ival = translate_constant("-", &(val[1]));
    else
        ival = translate_constant("+", val);

    return ival;
}


/*-----------------------------------------------------------------------*/
word translate_constant(char *sign, unsigned char *val)
{
    int ival;
    unsigned short lbl_value;

    /* val can either be a numeric constant, a character literal and a label */
    switch(val[0]){
      case '\'':             /* char literal */
        ival = (int) val[1];
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':              /* numerical value */
        ival = atoi(val);
        if(sign[0] == '-'){
            ival = -ival;
            if(ival < -(1 << (8*sizeof(short)-1))){  /* Underflow */
                sprintf(tmp_buf, "-%s: value too small", val);
                yyerror(tmp_buf);
            }
        }
        else{
            if(sign[0] != '+')
                INTERNAL_ERROR("in plus_minus rule.");
            if(ival >= (1 << (8*sizeof(short)))){  /* Overflow */
                strcpy(tmp_buf, val);
                yyerror(strcat(tmp_buf, ": value too large"));
            }
        }
        break;
      default:               /* label */
        if(!get_label(val, &lbl_value)){
            strcpy(tmp_buf, val);
            yyerror(strcat(tmp_buf, ": undefined identifier"));
        }
        ival = lbl_value;
    }

    return (word) ival;
}


/*-----------------------------------------------------------------------*/
void create_label(unsigned char *label, short value, int type)
{
    struct lbl *new_label;

    if((pass != 1) && (type != CONSTANT))
        return;

    if(get_label(label, (unsigned short *) &value)){
        sprintf(tmp_buf, "%s: identifier already defined.", label);
        yyerror(tmp_buf);
    }

#ifdef DEBUG1
    fprintf(stdout,"Creating label %s at address %d\n", label, value);
#endif
    new_label = (struct lbl *) malloc(sizeof(struct lbl));
    /* INICIO DAS MODIFICACOES lpc */
    strncpy(new_label->name, label, MAXLBL-1);
    new_label->name[MAXLBL-1] = '\0';
    if(strlen(label) >= MAXLBL){
    	fprintf(stderr, "Label %s (line %d) too long, truncated to %s.\n",
                label, line, new_label->name);
    }
    /* FIM DAS MODIFICACOES lpc */
    new_label->val = value;
    new_label->type = type;
    /* Insert at top of the list */
    if(lbl_base == NIL(struct lbl))
        new_label->next = NIL(struct lbl);
    else
        new_label->next = lbl_base;
    lbl_base = new_label;

    return;
}


/*-----------------------------------------------------------------------*/
int get_label(unsigned char *label, unsigned short *value)
{
    struct lbl *lbl_pos;

#ifdef DEBUG1
    fprintf(stdout,"Getting label %s.", label);
#endif

    lbl_pos = lbl_base;
    while(lbl_pos != NIL(struct lbl)){
        if(!strncmp(lbl_pos->name, label, MAXLBL-1)){  /* MODIFICADO lpc */
            *value = lbl_pos->val;
#ifdef DEBUG1
    fprintf(stdout," Got %d.\n", *value);
#endif
            return 1;
        }
        lbl_pos = lbl_pos->next;
    }
#ifdef DEBUG1
    fprintf(stdout," Didn't find it...\n");
#endif
    return 0;
}


/*-----------------------------------------------------------------------*/
void reverse_list()
{
    struct lbl *old,
               *tmp;

    if(lbl_base == NIL(struct lbl))
	return;

    old = lbl_base->next;
    lbl_base->next = NIL(struct lbl);
    while(old != NIL(struct lbl)){
        tmp = old;
        old = old->next;
        tmp->next = lbl_base;
        lbl_base = tmp;
    }
}


/*-----------------------------------------------------------------------*/
void dump_refs(FILE *lis)
{
    struct lbl *lbl_pos;
    char *type;

    /* So that the order is the same as in the source */
    reverse_list();

    fprintf(lis,"\n\t            G L O B A L   R E F E R E N C E S\n\n");
    fprintf(lis,"\t             Name               Value        Type\n");
    fprintf(lis,"\t==============================|=======|===============\n");
    lbl_pos = lbl_base;
    while(lbl_pos != NULL){
        switch(lbl_pos->type){
          case CONSTANT:
            type = "CONSTANT";
            break;
          case VAR_ADDR:
            type = "VARIABLE";
            break;
          case STR_ADDR:
            type = "STRING";
            break;
          case TAB_ADDR:
            type = "TABLE";
            break;
          case LABEL_ADDR:
            type = "LABEL";
            break;
          default:
            INTERNAL_ERROR("unkown label type.");
        }
        fprintf(lis, "\t%-30s   %4.4hX%17s\n",lbl_pos->name,lbl_pos->val,type);
        lbl_pos = lbl_pos->next;
    }
}

/*-------------------------------------------------------------------------
Counts the number of characters in a string delimited by '
+------------------------------------------------------------------------*/
int mystrlen(char *string)
{
    int n = 1;

    while(string[n++] != '\'');

    return n;
}

/*-------------------------------------------------------------------------
Concatenates 2 strings delimited by 's
+------------------------------------------------------------------------
void mystrcat(char *str1, char *str2)
{
    int i1 = 0,
        i2 = 0;

    while(str1[++i1] != '\'');

    while(str2[++i2] != '\'')
        str1[i1++] = str2[i2];
    str1[i1] = '\'';
}
*/

void memory_write(word val)
{
    if(mem_used[cur_addr])
	yyerror("Blocks of code overlap in memory!");
    else{
	mem_used[cur_addr] = 1;
	memory[cur_addr++] = val;
    }
}
