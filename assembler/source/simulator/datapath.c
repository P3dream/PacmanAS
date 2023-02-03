/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| The datapath.
|
| History: 17/mar/01 - JCM - created.
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                       Arlindo Oliveira, aml@inesc.pt
|                 	Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/

#include "defs_sim.h"

static int       function_unit(word, word, bitset, word *);
static word      rotate_left(word);
static word      rotate_right(word);
static word      rotate_left_carry(word, char *);
static word      rotate_right_carry(word, char *);


static word regs[16] ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};/* The register file */
static word RE;                   /* EZCNO flags for program */
static word uRE;                  /* zc flags for microprogram */


/*-------------------------------------------------------------------------
Relevant figure/table: Figures 2.1, Table #####
-------------------------------------------------------------------------*/
int datapath_computation(bitset F, bitset selAD, bitset selB, bitset MA,
			 bitset MB, bitset MD, bitset CALU, bitset WR,
			 bitset WM, bitset LE, bitset BME, bitset CONSTA,
			 bitset LI, word *IR, bitset *INTR)
{
    word opA,
         opB,
         result;
    int status;

    if(selAD > 15){
        fprintf(stderr,
		"Internal error! Invalid selAD field passed to datapath: %d\n", selAD);
	abort();
    }
    opA = regs[selAD];

    if(!F){
        if(selB > 15){
	    fprintf(stderr,
		    "Internal error! Invalid selB field passed to datapath: %d\n", selB);
	    abort();
	}
	opB = regs[selB];

	switch(MA){
	  case 0:
	    break;
	  case 1:
	    if(!F)
	        opA = opB;
	    break;
	  default:
	    fprintf(stderr,
		    "Internal error! Invalid MA field passed to datapath: %d\n", MA);
	    abort();
	}

        switch(MB){
	  case 0:
	    break;
	  case 1:
	    opB = *IR;
	    break;
	  default:
	    fprintf(stderr,
		    "Internal error! Invalid MB field passed to datapath: %d\n", MB);
	    abort();
	}
    }

    switch(MD){
      case 0:
	if(F && WR){
	    fprintf(stderr, "Microcode error! MD=0 with F=1!\n");
	    abort();
	}
	status = function_unit(opA, opB, CALU, &result);
        break;
      case 1:
        /* Terrible hack! Need to identify when the IVAD is being read.
           Assume this if R9<-M[], only happens here - BE CAREFUL WITH CHANGES*/
        if(selAD == 9){
            result = get_ivad();
            status = 0;
        }
        else
            status = read_main_memory(opA, &result);
        break;
      case 2:
	result = RE;
	status = 0;
        break;
      case 3:
	result = CONSTA;
	status = 0;
        break;
      default:
        fprintf(stderr,
                "Internal error! Invalid MD field passed to datapath: %x\n",MD);
        abort();
    }
    if(status)
        return status;

#ifdef DEBUG
printf("opA=%d opB=%d result=%d\n", opA, opB, result);
#endif

    if(!F){      /* Only perform these actions for type 0 microinstructions */
        RE = (RE & ~BME) | (uRE & BME);

	if(WM)
	    write_main_memory(opA, opB);
    }
    else{      /* Only perform these actions for type 1 microinstructions */
        if(LE)
	    RE = regs[selAD] & 0x1f;

	if(LI)
	    (void) read_main_memory(opA, IR);
    }

    /* Write result into register file */
    if(WR == 0)                          /* Do not update register file */
        return 0;
    if(selAD != 0)                          /* R[0] always 0! */
        regs[selAD] = result;

    return 0;
}


/*-------------------------------------------------------------------------
Relevant figure/table: Figures , Tables #####
-------------------------------------------------------------------------*/
static int function_unit(word opA, word opB, bitset CALU, word *result)
{
    char overflow,
         carry,
         old_carry = RE & (1 << CARRY);

    /* Pre-process the carry for arithmetic unit */
    if(old_carry)
        switch(CALU){
	  case 3:      /* A - B - /Cin */
	    CALU = 1;
	    break;
	  case 6:      /* A - 1 + Cin */
	    CALU = 24;
	    break;
	  case 7:      /* A + 1 - /Cin */
	    CALU = 2;
	    opB = 0;
	}
    else
        switch(CALU){
	  case 2:      /* A + B + Cin */
	    CALU = 0;
	    break;
	  case 6:      /* A - 1 + Cin */
	    CALU = 3;
	    opB = 0;
	    break;
	  case 7:      /* A + 1 - /Cin*/
	    CALU = 24;
	}

    switch(CALU){
      case 4:      /* A - 1 */
	CALU = 3;
	opB = 0;
	break;
      case 5:      /* A + 1 */
	CALU = 2;
	opB = 0;
    }

    overflow = 0;
    carry = 0;
    switch(CALU){
      case 2:                  /* A + B + 1 */
	if((opA != 0x7fff) && (opA != 0xffff))
	    opA++;
	else
	    if((opB != 0x7fff) && (opB != 0xffff))
	        opB++;
	    else{
                if(opA == 0x7fff){
	            if(opB == 0x7fff){
		        *result = opA + opB + 1;
			overflow = 1;
			carry = 0;
		    }
		    else{ /* opB = 0xffff */
		        *result = 0x7fff;
			overflow = 0;
			carry = 1;
		    }
		}
		else{ /* opA = 0xffff */
		    *result =  opB;
		    overflow = 0;
		    carry = 1;
		}
		break;
	    }
      case 0:                  /* A + B */    /* Continues previous case */
        *result = opA + opB;
        if(~(opA ^ opB) & 0x8000)       /* Sign A = Sign B */
            if((*result ^ opA) & 0x8000)   /* Sign A != Sign result */
                overflow = 1;
        if((opA & opB & 0x8000) ||      /* Sign A = Sign B = 1 */
           ((opA ^ opB) & (~*result) & 0x8000)) /*SignA!=SignB and Sign result=0*/
            carry = 1;
        break;
      case 3:                  /* A - B - 1 */
	if((opA != 0x8000) && (opA != 0x0000))
	    opA--;
	else
	    if((opB != 0x7fff) && (opB != 0xffff))
	        opB++;
	    else{
	        if(opB == 0x7fff)
		    if(opA == 0x8000){
		        *result = 0;
			overflow = 1;
			carry = 1;
		    }
		    else{  /* opA = 0x0000 */
		        *result = 0x8000;
			overflow = 0;
			carry = 0;
		    }
		else{  /* opB = 0xffff */
		    *result = opA;
		    overflow = 0;
		    carry = 0;
		}
		break;
            }
      case 1:                  /* A - B */    /* Continues previous case */
        *result = opA - opB;
        if((opA ^ opB) & 0x8000)        /* Sign A != Sign B */
            if((*result ^ opA) & 0x8000)   /* Sign A != Sign result */
                overflow = 1;
	if(((opA & (~opB) & 0x8000) ||      /* Sign A = 1 and Sign B = 0 */
	    ((~(opA ^ opB)) & (~*result) & 0x8000)))
	    carry = 1;      /* SgnA = SgnB and Sgn result = 0 */
//------------------------------------------------------------------------
// THIS WAS INCORRECT: it's coherent with adding the 2's complement of B,
// but it's is not how the carry is generated in a subtractor...
//       if((((~opA) & opB & 0x8000) ||      /* Sign A = 0 and Sign B = 1 */
//          ((~(opA ^ opB)) & *result & 0x8000)))
//          carry = 1;      /* SgnA = SgnB and Sgn result = 1 */
        break;
      case 8:
      case 12:
        *result = ~opA;
        break;
      case 9:
      case 13:
        *result = opA & opB;
        break;
      case 10:
      case 14:
        *result = opA | opB;
        break;
      case 11:
      case 15:
        *result = opA ^ opB;
        break;
      case 16:
        carry = opA & 1;
        *result = opA >> 1;
        break;
      case 17:
        carry = ((opA & 0x8000) != 0);
        *result = opA << 1;
        break;
      case 18:
        carry = opA & 1;
        *result = ((short int) opA) >> 1;
        break;
      case 19:
        carry = ((opA & 0x8000) != 0);
        *result = opA << 1;
        if((opA ^ *result) & 0x8000)   /* Sign A != Sign result */
            overflow = 1;
        break;
      case 20:
        carry = opA & 1;
        *result = rotate_right(opA);
        break;
      case 21:
        carry = ((opA & 0x8000) != 0);
        *result = rotate_left(opA);
        break;
      case 22:
        *result = rotate_right_carry(opA, &old_carry);
        carry = old_carry;
        break;
      case 23:
        *result = rotate_left_carry(opA, &old_carry);
        carry = old_carry;
        break;
      default:
	*result = opA;
    }

    uRE = 0;
    if(*result == 0)
        uRE |= (1 << uZERO);
    if((*result & 0x8000) != 0)
        uRE |= (1 << uNEGATIVE);
    if(overflow)
        uRE |= (1 << uOVERFLOW);
    if(carry)
        uRE |= (1 << uCARRY);

    return 0;
}


/*-----------------------------------------------------------------------*/
static word rotate_left(word operand)
{
    unsigned long resul;

    resul = operand;
    resul &= 0xffff;

    resul <<= 1;

    if(resul & 0x10000)
        resul |= 0x0001;
    resul &= 0xffff;

    return (word) resul;
}


/*-----------------------------------------------------------------------*/
static word rotate_right(word operand)
{
    unsigned long resul;

    resul = operand;
    resul &= 0xffff;

    if(resul & 0x0001)
        resul |= 0x10000;
    resul >>= 1;

    return (word) resul;
}


/*-----------------------------------------------------------------------*/
static word rotate_left_carry(word operand, char *carry)
{
    unsigned long resul;

    resul = operand;
    resul &= 0xffff;

    resul <<= 1;
    if(*carry)
        resul |= 1;
    else
        resul &= ~1;
    *carry = ((resul & 0x10000) != 0);

    resul &= 0xffff;

    return (word) resul;
}


/*-----------------------------------------------------------------------*/
static word rotate_right_carry(word operand, char *carry)
{
    unsigned long resul;

    resul = operand;
    resul &= 0xffff;

    if(*carry)
        resul |= 0x10000;
    *carry = (resul & 0x0001) ? 1 : 0;
    resul >>= 1;

    return (word) resul;
}


/*-------------------------------------------------------------------------
Tests the program flags, index defined in common.h
-------------------------------------------------------------------------*/
int test_RE(int index)
{
    if((index < 0) || (index > 4)){
        fprintf(stderr, "Internal error! Invalid RE flag test (index=%d).\n",
                index);
        abort();
    }

    return ((RE & (1 << index)) != 0);
}


/*-------------------------------------------------------------------------
Tests the micro-program flags, index defined in common.h
-------------------------------------------------------------------------*/
int test_uRE(int index)
{
    if((index < 0) || (index > 3)){
        fprintf(stderr, "Internal error! Invalid uRE flag test (index=%d).\n",
                index);
        abort();
    }

    return ((uRE & (1 << index)) != 0);
}


/*-------------------------------------------------------------------------
Called by interface routine.
-------------------------------------------------------------------------*/
int reg_content(int index, char *value)
{
    if((index < 0) || (index > 15))
        return 1;

    sprintf(value, "%4.4hx", regs[index]);

    return 0;
}


/*-------------------------------------------------------------------------
Called by interface routine.
-------------------------------------------------------------------------*/
int set_register(int index, word value)
{
    if((index < 0) || (index > 15))
        return 1;

    regs[index] = value;

    return 0;
}


/*-----------------------------------------------------------------------*/
int get_register(int index)
{
    if((index < 0) || (index > 15)){
        fprintf(stderr,
                "Internal error! Invalid register index passed to datapath: %d\n", index);
        abort();
    }

    return regs[index];
}


/*-------------------------------------------------------------------------*/
void set_RE(mem_addr addr)
{
    RE = addr;
}
