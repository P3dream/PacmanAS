/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Definitions for the simulator.
|
| History: 17/mar/01 - JCM - created.
|           2/nov/02 - JCM - new instruction format.
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                       Arlindo Oliveira, aml@inesc.pt
|                 	Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/


#include "common.h"

/* Data types */
typedef unsigned short int bitset;       /* For storage of sets of bits */
typedef unsigned char reg_index;         /*Index for register in register file*/


/* Microinstructions fields */
#define getF(uI)     ((bitset)((0x80000000 & (uI)) >> 31))
#define getM5(uI)    ((bitset)((0x60000000 & (uI)) >> 29))
#define getSR1(uI)   ((bitset)((0x10000000 & (uI)) >> 28))
#define getSR2(uI)   ((bitset)((0x08000000 & (uI)) >> 27))
#define getWR(uI)    ((bitset)((0x00000080 & (uI)) >> 7))
#define getMD(uI)    ((bitset)((0x00000060 & (uI)) >> 5))
#define getMAD(uI)   ((bitset)((0x00000010 & (uI)) >> 4))
#define getRAD(uI)   ((bitset) (0x0000000f & (uI)))
/* Type 0 */
#define getIAK(uI)   ((bitset)((0x04000000 & (uI)) >> 26))
#define getBME(uI)   ((bitset)((0x03c00000 & (uI)) >> 22))
#define getCALU(uI)  ((bitset)((0x003e0000 & (uI)) >> 17))
#define getMA(uI)    ((bitset)((0x00010000 & (uI)) >> 16))
#define getMB(uI)    ((bitset)((0x00008000 & (uI)) >> 15))
#define getM2(uI)    ((bitset)((0x00004000 & (uI)) >> 14))
#define getMRB(uI)   ((bitset)((0x00002000 & (uI)) >> 13))
#define getRB(uI)    ((bitset)((0x00001e00 & (uI)) >> 9))
#define getWM(uI)    ((bitset)((0x00000100 & (uI)) >> 8))
/* Type 1 */
#define getLS(uI)    ((bitset)((0x04000000 & (uI)) >> 26))
#define getMCOND(uI) ((bitset)((0x03800000 & (uI)) >> 23))
#define getCC(uI)    ((bitset)((0x00400000 & (uI)) >> 22))
#define getLI(uI)    ((bitset)((0x00200000 & (uI)) >> 21))
#define getLE(uI)    ((bitset)((0x00100000 & (uI)) >> 20))
#define getCONST(uI) ((bitset)((0x000fff00 & (uI)) >> 8))


/* Instruction fields */
#define getOP(I)      ((0xfc00 & (I)) >> 10)
#define getOP54(I)    ((0xc000 & (I)) >> 14)
#define getOP30(I)    ((0x3c00 & (I)) >> 10)
#define getS(I)       ((0x0200 & (I)) >> 9)
#define getREG2(I)    ((0x01c0 & (I)) >> 6)
#define getMODE(I)    ((0x0030 & (I)) >> 4)
#define getREG1(I)     (0x000f & (I))
#define getARG_0OP(I)  (0x03ff & (I))
#define getSHAM(I)    ((0x03c0 & (I)) >> 6)
#define getCOND(I)    ((0x03c0 & (I)) >> 6)
#define getOFFSET(I)   (0x003f & (I))



/* Generic bit manipulation routines */
#define EQUAL_BIT(from, to, pos) ((to) = (((from) & (1 << (pos))) | ((to) & (~(1 << (pos))))))
#define COPY_BIT(bit, to, pos) ((to) = (bit) ? ((to) | (1 << (pos))) : ((to) & (~(1 << (pos)))))


/* Initial values of registers */
#define INITIAL_CAR 0x000
#define INITIAL_PC  0x0000


/* Address where the interrupt vector table begins */
#define IVT_ADDR 0xfe00

#define	TIMER_UNITS	0xfff6	 /* port to read/write the number of time units */   
#define	TIMER_START	0xfff7	 /* port to start/stop the countdown */

#define MASK_ADDR 	0xfffa	 /* port to read/write the interrupt mask */

#define IO_TEXT_READ	0xffff	 /* read key form text window */
#define IO_TEXT_WRITE	0xfffe	 /* write character to text window */
#define IO_TEXT_STATUS	0xfffd	 /* indicates whether input is available */
#define IO_TEXT_CONTROL	0xfffc   /* positions cursor in lcd window */

#define IO_LCD_WRITE	0xfff5   /* write character to lcd window */ 
#define IO_LCD_CONTROL	0xfff4   /* positions cursor in lcd window */


#define IO_SWITCHES     0xfff9   /* port to read the 8 switches */

#define IO_LEDS         0xfff8   /* port to write to the 8 leds */

#define IO_SEVEN_SEG3   0xfff3   /* port for seven segments led */
#define IO_SEVEN_SEG2   0xfff2   /* port for seven segments led */
#define IO_SEVEN_SEG1   0xfff1   /* port for seven segments led */
#define IO_SEVEN_SEG0   0xfff0   /* port for seven segments led */

#define VALUE_READ_FROM_WRITE_PORTS 0xffff


int datapath_computation(bitset, bitset, bitset, bitset, bitset, bitset, bitset,
                         bitset, bitset, bitset, bitset, bitset, bitset, word *,
                         bitset *);
                                        /* datapath.c */
int  get_register(int);                 /* datapath.c */
void set_RE(mem_addr);                  /* datapath.c */
int  test_RE(int);                      /* datapath.c */
int  test_uRE(int);                     /* datapath.c */

word get_ivad();                        /* control.c */

dword read_control_ROM(ctrlROM_addr);   /* microcode.c */

word io_read_switches();		/* commands.c */
word read_start();			/* commands.c */
word read_mask();			/* commands.c */
word read_units();			/* commands.c */
void refresh(mem_addr);                 /* commands.c */
void writeIOText(char, mem_addr);	/* commands.c */
void writeLCDText(char, mem_addr);	/* commands.c */
void writeLEDs(mem_addr);		/* commands.c */
void write7seg(int, mem_addr);		/* commands.c */
void writeMASK(mem_addr);		/* commands.c */
void writeTIMER(char, mem_addr);	/* commands.c */
void time_ivec();			/* commands.c */
void nothing();				/* commands.c */
