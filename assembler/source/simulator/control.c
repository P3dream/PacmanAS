/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2007
|
| Control Unit.
|
| History: 17/mar/01 - JCM - created.
|          18/fev/11 - JCM - swapped P/NP and E/NE codes, to conform with docs
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                       Arlindo Oliveira, aml@inesc.pt
|                 	Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/

#include "defs_sim.h"


static ctrlROM_addr handle_ROMB(bitset, bitset);
static bitset       compute_condition(bitset, bitset);

static ctrlROM_addr initial_CAR =INITIAL_CAR;    /* Holds initial CAR after reset */

static dword microinstruction;    /* Holds current micro-instruction */
static word IR;                   /* Register that holds current instruction */
static ctrlROM_addr CAR;          /* Register that holds the address of u-instruction */
static ctrlROM_addr SBR;          /* Holds the return address from micro-subroutine */
static bitset INTR = 0;           /* Indicates an external interrupt pending */
static ivad_index IVAD[N_IVAD];   /* Holds pending interrupt vectors */



/*-----------------------------------------------------------------------*/
void reset_uP()
{
    int i;
    char command[16];

    CAR = initial_CAR;
    INTR = 0;
    microinstruction = read_control_ROM(CAR);

    for(i = 1; i < 15; i++)
	set_register(i, 0);
    set_register(PC_R15, INITIAL_PC);
    set_RE(0);

    code_unassemble();
}


/*-------------------------------------------------------------------------
Relevant figure/table: #####
Returned value indicates if there was an error.
-------------------------------------------------------------------------*/
int execute1clock_cycle()
{
    bitset F, M5, SR1, SR2, WR, MD, MAD, RAD,
           IAK=-1, BME=-1, CALU=-1, MA=-1, MB=-1, M2=-1, MRB=-1, RB=-1, WM=-1,
           LS=-1, MCOND=-1, CC=-1, LI=-1, LE=-1, CONST_NA=-1;
    word selAD, selB;
    bitset cond;
    int status;

    /* microinstruction should already have been updated */
    F = getF(microinstruction);
    M5 = getM5(microinstruction);
    SR1 = getSR1(microinstruction);
    SR2 = getSR2(microinstruction);
    WR = getWR(microinstruction);
    MD = getMD(microinstruction);
    MAD = getMAD(microinstruction);
    RAD = getRAD(microinstruction);

    if(F){                           /* type 1: BR/CALL or Reg<-CONST*/
	LS = getLS(microinstruction);
	MCOND = getMCOND(microinstruction);
	CC = getCC(microinstruction);
	LI = getLI(microinstruction);
	LE = getLE(microinstruction);
	CONST_NA = getCONST(microinstruction);

#ifdef DEBUG
printf("CAR=%x\nF=%x M5=%x SR1=%x SR2=%x LS=%x MCOND=%x CC=%x LI=%x LE=%x\nCONST=%x WR=%x MD=%x MAD=%x RAD=%x\n",
CAR, F, M5, SR1, SR2, LS, MCOND, CC, LI, LE, CONST_NA, WR, MD, MAD, RAD);
#endif

    }
    else{
        IAK = getIAK(microinstruction);
	BME = getBME(microinstruction);
	CALU = getCALU(microinstruction);
	MA = getMA(microinstruction);
	MB = getMB(microinstruction);
	M2 = getM2(microinstruction);
	MRB = getMRB(microinstruction);
	RB = getRB(microinstruction);
        WM = getWM(microinstruction);

#ifdef DEBUG
printf("CAR=%x\nF=%x M5=%x SR1=%x SR2=%x IAK=%x BME=%x CALU=%x\nMA=%x MB=%x M2=%x MRB=%x RB=%x WM=%x WR=%x MD=%x MAD=%x RAD=%x\n",
CAR, F, M5, SR1, SR2, IAK, BME, CALU, MA, MB, M2, MRB, RB, WM, WR, MD, MAD, RAD);
#endif
    }

    /* Execute microinstruction and compute new CAR */

    /* New CAR */
    if(F && LS)
        SBR = CAR + 1;
    switch(M5){
      case 0:                          /* NXT */
	if(!F)
	    CAR++;
	else{
	    cond = compute_condition(MCOND, CC);
	    CAR = cond ? CONST_NA : CAR + 1;
	}
	break;
      case 1:                          /* RET */
	CAR = SBR;
	break;
      case 2:                          /* MAP Opcode */
	CAR = read_ROMA(getOP(IR));
	break;
      case 3:                          /* MAP Mode */
	CAR = handle_ROMB(SR1, SR2);
	break;
      default:
        fprintf(stderr,
                "Internal error! Invalid M5 field passed to control: %x\n",M5);
        abort();
    }

    /* Get operands */
    selB = MRB ? RB : (M2 ? getREG2(IR) : getREG1(IR));
    selAD = MAD ? RAD : ((getS(IR) ^ M2) && (getOP(IR) & 0x20) ?
			                         getREG2(IR) : getREG1(IR));

    /* Execution Unit */
    status = datapath_computation(F, selAD, selB, MA, MB, MD, CALU, WR, WM,
				  LE, BME, CONST_NA, LI, &IR, &INTR);
    if(status)
        return status;

    /* Update microinstruction for new CAR */
    microinstruction = read_control_ROM(CAR);

    return 0;
}


/*-------------------------------------------------------------------------
Relevant figure/table: Table ####, Figure ####
-------------------------------------------------------------------------*/
static ctrlROM_addr handle_ROMB(bitset SR1, bitset SR2)
{
    word mapROM_addr;

    mapROM_addr = getMODE(IR);
    if(SR2)
        if(getS(IR))
	    mapROM_addr |= 0xc;
	else
	    mapROM_addr |= 0x8;
    else
        if(SR1)
	    mapROM_addr |= 0x4;

    return read_ROMB(mapROM_addr);
}


/*-------------------------------------------------------------------------
Relevant figure/table: Table #####, Figure ####
-------------------------------------------------------------------------*/
static bitset compute_condition(bitset MCOND, bitset CC)
{
    bitset condition;

    /* Get variable to test from MCOND */
    switch(MCOND){
      case 0:
        condition = 1;
        break;
      case 1:
        condition = test_uRE(uZERO);
        break;
      case 2:
        condition = test_uRE(uCARRY);
        break;
      case 3:
        condition = INTR & test_RE(INT_EN);
        break;
      case 4:
        condition = getMODE(IR) & 1;
        break;
      case 5:
        condition = getMODE(IR) & 2;
        break;
      case 6:      /* Bit S only makes sense for 2 operand instructions */
        condition = getS(IR) & (getOP54(IR) == 2);
        break;
      case 7:
	switch((getCOND(IR) & 0xe) >> 1){
	  case 0:
	    condition = test_RE(ZERO);
	    break;
	  case 1:
	    condition = test_RE(CARRY);
	    break;
	  case 2:
	    condition = test_RE(NEGATIVE);
	    break;
	  case 3:
	    condition = test_RE(OVERFLOW);
	    break;
	  case 4:
	    condition = ((test_RE(ZERO) || test_RE(NEGATIVE)) == 0);
	    break;
	  case 5:
	    condition = INTR;
	    break;
	  case 6:
	    condition = test_RE(INT_EN);
	    break;
	  default:
	    fprintf(stderr, "Internal error! Invalid BRANCH CONDITION: %x\n",
		    getCOND(IR));
	    abort();
	}
        condition ^= getCOND(IR) & 1;
	break;
      default:
	fprintf(stderr, "Internal error! Invalid MCOND field in control: %x\n",
		MCOND);
	abort();
    }

    return condition ^ CC;
}


/*-------------------------------------------------------------------------*/
void interrupt(ivad_index addr)
{
    if(addr >= N_IVAD){
	fprintf(stderr, "Internal error! IVAD out of range!\n");
	abort();
    }
    IVAD[addr] = 1;
    INTR = 1;
}


/*-------------------------------------------------------------------------
JCM may/05: Timer means that interrupts from different sources may be
            pending. Return the one with larger priority and only reset
	    INTR if there is no other one pending.
-------------------------------------------------------------------------*/
word get_ivad()
{
    int iv,
	i;

    for(i = N_IVAD - 1; i >= 0; i--)
	if(IVAD[i]){
	    IVAD[i] = 0;
	    iv = i;
	    break;
	}
    if(i < 0){
	fprintf(stderr, "Internal error! INTR=1 and no interrupt pending!\n");
	abort();
    }
    INTR = 0;
    while(i >= 0)
	if(IVAD[i--])
	    INTR = 1;
    return iv;
}


/*-------------------------------------------------------------------------
Called by interface routine.
-------------------------------------------------------------------------*/
void pc_content(char *value)
{
    sprintf(value, "%4.4hx", get_register(PC_R15));
}


/*-------------------------------------------------------------------------
Called by interface routine.
-------------------------------------------------------------------------*/
void sp_content(char *value)
{
    sprintf(value, "%4.4hx", get_register(SP_R14));
}


/*-------------------------------------------------------------------------
Called by interface routine.
-------------------------------------------------------------------------*/
ctrlROM_addr car_content()
{
    return CAR;
}


/*-------------------------------------------------------------------------
Called by interface routine.
-------------------------------------------------------------------------*/
void ir_content(char *value)
{
    sprintf(value, "%4.4hx", IR);
}


/*-------------------------------------------------------------------------
Called by interface routine.
-------------------------------------------------------------------------*/
void sbr_content(char *value)
{
    sprintf(value, "%3.3hx", SBR);
}


/*-------------------------------------------------------------------------
Called by interface routine.
-------------------------------------------------------------------------*/
void ui_content(char *value)
{
    sprintf(value, "%8.8lx", microinstruction);
}


/*-------------------------------------------------------------------------
Called by interface routine.
-------------------------------------------------------------------------*/
void int_content(char *value)
{
    sprintf(value, "%c", INTR ? '1' : '0');
}
