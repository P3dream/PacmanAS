/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Unassembles the code.
|
| History: 17/mar/01 - JCM - created.
|           4/nov/02 - JCM - new opcodes for control instructions
|                            new format for BR
|          18/fev/11 - JCM - swapped P/NP and E/NE codes, to conform with docs
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                       Arlindo Oliveira, aml@inesc.pt
|                 	Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/


#include "defs_sim.h"

#define DISPLACEMENT(mem) ((mem) < 0 ? "-" : "+")


static void decode_instruction(mem_addr *);
static void decode_0op(word, mem_addr *);
static void decode_1op(word, mem_addr *);
static void decode_2op(word, mem_addr *);
static void decode_branch(word, mem_addr *);
static void decode_cond_branch(char *, char *, int);
static int  decode_br_offset(word, char []);
static int  decode_operand(word, mem_addr, char []);


static char *unassembled_inst[MAIN_MEM_SIZE];  /* array with unassembled inst */
static char inst_tmp[LINE_LENGTH];             /* just for temp. storage */



/*-------------------------------------------------------------------------

-------------------------------------------------------------------------*/
void init_code_unassemble()
{
    mem_addr addr = 0;

    do{
        unassembled_inst[addr] = NIL(char);
        addr++;
    } while(addr != 0);
}


/*-----------------------------------------------------------------------*/
void code_unassemble()
{
    mem_addr addr = 0;

    do{   /* First make sure all memory positions are set to NIL */
        unassembled_inst[addr] = NIL(char);
        addr++;
    } while(addr != 0);
    do{
        decode_instruction(&addr);
    } while(addr < IVT_ADDR);
}


/*-----------------------------------------------------------------------*/
static void decode_instruction(mem_addr *addr)
{
    word inst;

    read_main_memory(*addr, &inst);

    switch(getOP54(inst)){
      case 0:
        decode_0op(inst, addr);
        break;
      case 1:
        decode_1op(inst, addr);
        break;
      case 2:
        decode_2op(inst, addr);
        break;
      case 3:
        decode_branch(inst, addr);
    }
}


/*-----------------------------------------------------------------------*/
static void decode_0op(word inst, mem_addr *addr)
{
    static char *inst_0op[] = {"NOP", "ENI", "DSI", "STC", "CLC", "CMC",
			       "RET", "RTI",
			       "INT", "RETN",
			       "???", "???", "???", "???", "???", "I0OP"};
    unsigned short opcode;

    opcode = getOP30(inst);

    if(opcode < 8){
        sprintf(inst_tmp, "%4.4hx    %s", *addr, inst_0op[opcode]);
        unassembled_inst[*addr] = strdup(inst_tmp);
    }
    else
        if(opcode < 10){
	    sprintf(inst_tmp, "%4.4hx    %-7s %hx", *addr, inst_0op[opcode],
		    getARG_0OP(inst));
	    unassembled_inst[*addr] = strdup(inst_tmp);
	}
	else{
	    sprintf(inst_tmp, "%4.4hx    %s", *addr, inst_0op[opcode]);
	    unassembled_inst[*addr] = strdup(inst_tmp);
	}

    (*addr)++;
}


/*-----------------------------------------------------------------------*/
static void decode_1op(word inst, mem_addr *addr)
{
    static char *inst_1op[] = {"NEG", "INC", "DEC", "COM", "PUSH", "POP",
			       "SXT",  /* "MUL10",*/
			       "I1OP", /* "LOOP", "RND",  */
			       "SHR", "SHL", "SHRA", "SHLA", "ROR", "ROL",
                               "RORC", "ROLC"};
    static char operand[LINE_LENGTH];
    int opcode,
        incr = 1;

    opcode = getOP30(inst);
    incr = decode_operand(inst, *addr, operand);

    if(opcode < 8)
        sprintf(inst_tmp, "%4.4hx    %-7s %s", *addr, inst_1op[opcode], operand);
    else
        sprintf(inst_tmp, "%4.4hx    %-7s %s, %hx", *addr, inst_1op[opcode],
		operand, getSHAM(inst));

    unassembled_inst[*addr] = strdup(inst_tmp);

    (*addr) += incr;
}


/*-----------------------------------------------------------------------*/
static void decode_2op(word inst, mem_addr *addr)
{
    static char *inst_2op[] = {"CMP", "ADD", "ADDC", "SUB", "SUBB",
                               "MUL", "DIV", "TEST", "AND", "OR", "XOR",
                               "MOV", "MVBH", "MVBL", "XCH", "I2OP"};
    static char operand[LINE_LENGTH];
    int opcode,
        incr = 1;

    opcode = getOP30(inst);
    incr = decode_operand(inst, *addr, operand);

    if(opcode < 16){
        if(getS(inst))
            sprintf(inst_tmp, "%4.4hx    %-7s R%d, %s", *addr, inst_2op[opcode],
                    getREG2(inst), operand);
        else
	    sprintf(inst_tmp, "%4.4hx    %-7s %s, R%d", *addr, inst_2op[opcode],
                    operand, getREG2(inst));
    }
    else
        sprintf(inst_tmp, "%4.4hx    ???", *addr);

    unassembled_inst[*addr] = strdup(inst_tmp);

    (*addr) += incr;
}


/*-----------------------------------------------------------------------*/
static void decode_branch(word inst, mem_addr *addr)
{
    static char op[LINE_LENGTH],
                target[LINE_LENGTH];
    int opcode,
        condition,
        incr = 1;

    opcode = getOP30(inst);
    condition = getCOND(inst);

    switch(opcode){
      case 0:
        sprintf(op, "JMP");
	incr = decode_operand(inst, *addr, target);
        break;
      case 1:
        decode_cond_branch(op, "JMP", condition);
	incr = decode_operand(inst, *addr, target);
	break;
      case 2:
        sprintf(op, "CALL");
	incr = decode_operand(inst, *addr, target);
        break;
      case 3:
        decode_cond_branch(op, "CALL", condition);
	incr = decode_operand(inst, *addr, target);
	break;
      case 8:
        sprintf(op, "BR");
	incr = decode_br_offset(inst, target);
        break;
      case 9:
        decode_cond_branch(op, "BR", condition);
	incr = decode_br_offset(inst, target);
	break;
      case 15:
        sprintf(op, "ICTL");
	incr = decode_operand(inst, *addr, target);
	break;
      default:
        sprintf(op, "???");
        (*addr) += 1;
        return;
    }

    sprintf(inst_tmp, "%4.4hx    %-7s %s", *addr, op, target);

    unassembled_inst[*addr] = strdup(inst_tmp);

    (*addr) += incr;
}


/*-----------------------------------------------------------------------*/
static void decode_cond_branch(char *op, char *inst, int condition)
{
    switch(condition){
      case 0:
        sprintf(op, "%s.Z", inst);
	return;
      case 1:
	sprintf(op, "%s.NZ", inst);
        return;
      case 2:
	sprintf(op, "%s.C", inst);
        return;
      case 3:
	sprintf(op, "%s.NC", inst);
        return;
      case 4:
	sprintf(op, "%s.N", inst);
        return;
      case 5:
	sprintf(op, "%s.NN", inst);
        return;
      case 6:
	sprintf(op, "%s.O", inst);
        return;
      case 7:
	sprintf(op, "%s.NO", inst);
        return;
      case 8:
	sprintf(op, "%s.P", inst);
        return;
      case 9:
	sprintf(op, "%s.NP", inst);
        return;
      case 10:
	sprintf(op, "%s.I", inst);
        return;
      case 11:
	sprintf(op, "%s.NI", inst);
        return;
      case 12:
	sprintf(op, "%s.E", inst);
        return;
      case 13:
	sprintf(op, "%s.NE", inst);
        return;
      default:
	sprintf(op, "???");
    }
}


/*-----------------------------------------------------------------------*/
static int decode_operand(word inst, mem_addr addr, char operand[])
{
    char reg;
    short int mem;

    reg = getREG1(inst);
    read_main_memory(addr+1, &mem);

    switch(getMODE(inst)){
      case 0:
	switch(reg){
	  case 15:
	    sprintf(operand, "PC");
	    return 1;
	  case 14:
	    sprintf(operand, "SP");
	    return 1;
	  default:
	    sprintf(operand, "R%d", reg);
	    return 1;
	}
      case 1:
	switch(reg){
	  case 15:
	    sprintf(operand, "M[PC]");
	    return 1;
	  case 14:
	    sprintf(operand, "M[SP]");
	    return 1;
	  default:
	    sprintf(operand, "M[R%d]", reg);
	    return 1;
	}
      case 2:
        sprintf(operand, "%4.4hx", mem);
        return 2;
      case 3:
	switch(reg){
	  case 15:
	    sprintf(operand, "M[PC%s%x]", DISPLACEMENT(mem), ABS(mem));
	    return 2;
	  case 14:
	    sprintf(operand, "M[SP%s%x]", DISPLACEMENT(mem), ABS(mem));
	    return 2;
	  case 0:
	    sprintf(operand, "M[%4.4hx]", mem);
	    return 2;
	  default:
	    sprintf(operand, "M[R%d%s%x]", reg, DISPLACEMENT(mem), ABS(mem));
	    return 2;
	}
    }
}


/*-----------------------------------------------------------------------*/
static int decode_br_offset(word inst, char offset[])
{
    char br_offset;

    br_offset = getOFFSET(inst);

    if(br_offset & 0x20){
	br_offset |= 0xc0;
	sprintf(offset, "-%d", -br_offset);
    }
    else
	sprintf(offset, "%d", br_offset);

    return 1;     /* BR instructions always occupy a single position */
}


/*-----------------------------------------------------------------------*/
char *get_instruction(mem_addr addr)
{
    return unassembled_inst[addr];
}
