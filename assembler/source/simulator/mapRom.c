/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Mapping ROMs.
|
| History: 17/mar/01 - JCM - created.
|           4/nov/02 - JCM - new addresses
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                       Arlindo Oliveira, aml@inesc.pt
|                 	Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/


#include "defs_sim.h"


static ctrlROM_addr mapROM_A[N_POS_ROM_MAP_A] = {
/*   0 - 000000 */ 0x032,      /* NOP     */
/*   1 - 000001 */ 0x033,      /* ENI0    */
/*   2 - 000010 */ 0x037,      /* DSI0    */
/*   3 - 000011 */ 0x03b,      /* STC0    */
/*   4 - 000100 */ 0x03e,      /* CLC0    */
/*   5 - 000101 */ 0x040,      /* CMC0    */
/*   6 - 000110 */ 0x044,      /* RET0    */
/*   7 - 000111 */ 0x047,      /* RTI0    */
/*   8 - 001000 */ 0x04c,      /* INT0    */
/*   9 - 001001 */ 0x055,      /* RETN0   */
/*  10 - 001010 */ 0x0,        /* Unused  */
/*  11 - 001011 */ 0x0,        /* Unused  */
/*  12 - 001100 */ 0x0,        /* Unused  */
/*  13 - 001101 */ 0x0,        /* Unused  */
/*  14 - 001110 */ 0x0,        /* Unused  */
/*  15 - 001111 */ 0x0,        /* Unused  */
/*  16 - 010000 */ 0x05b,      /* NEG0    */
/*  17 - 010001 */ 0x05e,      /* INC0    */
/*  18 - 010010 */ 0x060,      /* DEC0    */
/*  19 - 010011 */ 0x062,      /* COM0    */
/*  20 - 010100 */ 0x064,      /* PUSH0   */
/*  21 - 010101 */ 0x067,      /* POP0    */
/*  22 - 010110 */ 0x0,        /* Unused  */
/*  23 - 010111 */ 0x0,        /* Unused  */
/*  24 - 011000 */ 0x06a,      /* SHR0    */
/*  25 - 011001 */ 0x071,      /* SHL0    */
/*  26 - 011010 */ 0x078,      /* SHRA0   */
/*  27 - 011011 */ 0x07f,      /* SHLA0   */
/*  28 - 011100 */ 0x08c,      /* ROR0    */
/*  29 - 011101 */ 0x093,      /* ROL0    */
/*  30 - 011110 */ 0x09a,      /* RORC0   */
/*  31 - 011111 */ 0x0a1,      /* ROLC0   */
/*  32 - 100000 */ 0x0c2,      /* CMP0    */
/*  33 - 100001 */ 0x0b4,      /* ADD0    */
/*  34 - 100010 */ 0x0b6,      /* ADDC0   */
/*  35 - 100011 */ 0x0b8,      /* SUB0    */
/*  36 - 100100 */ 0x0ba,      /* SUBB0   */
/*  37 - 100101 */ 0x0cf,      /* MUL0    */
/*  38 - 100110 */ 0x0dd,      /* DIV0    */
/*  39 - 100111 */ 0x0c4,      /* TEST0   */
/*  40 - 101000 */ 0x0bc,      /* AND0    */
/*  41 - 101001 */ 0x0be,      /* OR0     */
/*  42 - 101010 */ 0x0c0,      /* XOR0    */
/*  43 - 101011 */ 0x0a8,      /* MOV0    */
/*  44 - 101100 */ 0x0af,      /* MVBH0   */
/*  45 - 101101 */ 0x0aa,      /* MVBL0   */
/*  46 - 101110 */ 0x0ca,      /* XCH0    */
/*  47 - 101111 */ 0x0,        /* Unused  */
/*  48 - 110000 */ 0x102,      /* JMP0    */
/*  49 - 110001 */ 0x105,      /* JMP.C0  */
/*  50 - 110010 */ 0x109,      /* CALL0   */
/*  51 - 110011 */ 0x10d,      /* CALL.C0 */
/*  52 - 110100 */ 0x0,        /* Unused  */
/*  53 - 110101 */ 0x0,        /* Unused  */
/*  54 - 110110 */ 0x0,        /* Unused  */
/*  55 - 110111 */ 0x0,        /* Unused  */
/*  56 - 111000 */ 0x0f9,      /* BR0     */
/*  57 - 111001 */ 0x0f8,      /* BR.C0   */
/*  58 - 111010 */ 0x0,        /* Unused  */
/*  59 - 111011 */ 0x0,        /* Unused  */
/*  60 - 111100 */ 0x0,        /* Unused  */
/*  61 - 111101 */ 0x0,        /* Unused  */
/*  62 - 111110 */ 0x0,        /* Unused  */
/*  63 - 111111 */ 0x0         /* Unused  */
};


/*-----------------------------------------------------------------------*/
ctrlROM_addr read_ROMA(word ROMaddress)
{
    return mapROM_A[ROMaddress];
}


/*-----------------------------------------------------------------------*/
void write_ROMA(word ROMaddress, ctrlROM_addr data)
{
    mapROM_A[ROMaddress] = data;
}



static ctrlROM_addr mapROM_B[N_POS_ROM_MAP_B] = {
/*   0 - 0000 */ 0x00a,        /* F1R0   */
/*   1 - 0001 */ 0x00b,        /* F1RI0  */
/*   2 - 0010 */ 0x00d,        /* F1IM0  */
/*   3 - 0011 */ 0x00f,        /* F1IN0  */
/*   4 - 0100 */ 0x02d,        /* WBR0   */
/*   5 - 0101 */ 0x02f,        /* WBM0   */
/*   6 - 0110 */ 0x02d,        /* WBR0   */
/*   7 - 0111 */ 0x02f,        /* WBM0   */
/*   8 - 1000 */ 0x013,        /* F2R0   */
/*   9 - 1001 */ 0x017,        /* F2RI0  */
/*  10 - 1010 */ 0x01d,        /* F2IM0  */
/*  11 - 1011 */ 0x023,        /* F2IN0  */
/*  12 - 1100 */ 0x015,        /* F2RS0  */
/*  13 - 1101 */ 0x01a,        /* F2RIS0 */
/*  14 - 1110 */ 0x020,        /* F2IMS0 */
/*  15 - 1111 */ 0x028         /* F2INS0 */
};


/*-----------------------------------------------------------------------*/
ctrlROM_addr read_ROMB(word ROMaddress)
{
    return mapROM_B[ROMaddress];
}


/*-----------------------------------------------------------------------*/
void write_ROMB(word ROMaddress, ctrlROM_addr data)
{
    mapROM_B[ROMaddress] = data;
}
