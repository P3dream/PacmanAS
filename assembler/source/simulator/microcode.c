/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Control ROM.
|
| History: 17/mar/01 - JCM - created.
|           4/nov/02 - JCM - new instruction format
|                            IH: intaddr is now an entry in table
|                            SH/ROTS: 16 positions was not working, removed
|                            SHLA: fixed overflow flag problem
|                            MUL: fixed problem of E flag becoming 0
|                            BR: offset now part of the instruction word
|                            OpFetch now only uses one register for address
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                       Arlindo Oliveira, aml@inesc.pt
|                 	Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/


#include "defs_sim.h"

/* Contents of the control ROM: 512 positions with 32 bits each. */

static dword controlROM [N_POS_ROM_CONTROL] = {
/* 000 000h 000000000b */  0x8060001f,  /* IF0      IR<-M[PC]                         */
/* 001 001h 000000001b */  0x400a009f,  /* IF1      PC<-PC+1, CAR<-ROMA[OP]           */
/* 002 002h 000000010b */  0x81c000d8,  /* IH0      R8<-RE, !EINT?CAR<-IF0            */
/* 003 003h 000000011b */  0x0008319e,  /* IH1      M[SP]<-R8, SP<-SP-1               */
/* 004 004h 000000100b */  0x04083f9e,  /* IH2      M[SP]<-PC, SP<-SP-1, IAK<-1       */
/* 005 005h 000000101b */  0x000000b9,  /* IH3      R9<-INTADDR                       */
/* 006 006h 000000110b */  0x804200f8,  /* IH4      R8<-0200h                         */
/* 007 007h 000000111b */  0x00023099,  /* IH5      R9<-R9-R8                         */
/* 008 008h 000001000b */  0x000132bf,  /* IH6      PC<-M[R9]                         */
/* 009 009h 000001001b */  0x80100010,  /* IH7      RE<-R0, CAR<-IF0                  */
/* 010 00ah 000001010b */  0x2031009d,  /* F1R0     RD<-R[IR1], CAR<-SBR              */
/* 011 00bh 000001011b */  0x0031009c,  /* F1RI0    EA<-R[IR1]                        */
/* 012 00ch 000001100b */  0x200138bd,  /* F1RI1    RD<-M[EA], CAR<-SBR               */
/* 013 00dh 000001101b */  0x00013ebd,  /* F1IM0    RD<-M[PC]                         */
/* 014 00eh 000001110b */  0x200a009f,  /* F1IM1    PC<-PC+1, CAR<-SBR                */
/* 015 00fh 000001111b */  0x00013ebc,  /* F1IN0    EA<-M[PC]                         */
/* 016 010h 000010000b */  0x000a009f,  /* F1IN1    PC<-PC+1                          */
/* 017 011h 000010001b */  0x0000009c,  /* F1IN2    EA<-EA+R[IR1]                     */
/* 018 012h 000010010b */  0x200138bd,  /* F1IN3    RD<-M[EA], CAR<-SBR               */
/* 019 013h 000010011b */  0x0031009d,  /* F2R0     RD<-R[IR1]                        */
/* 020 014h 000010100b */  0x2031409b,  /* F2R1     SD<-R[IR2], CAR<-SBR              */
/* 021 015h 000010101b */  0x0031009b,  /* F2RS0    SD<-R[IR1]                        */
/* 022 016h 000010110b */  0x2031409d,  /* F2RS1    RD<-R[IR2], CAR<-SBR              */
/* 023 017h 000010111b */  0x0031009c,  /* F2RI0    EA<-R[IR1]                        */
/* 024 018h 000011000b */  0x000138bd,  /* F2RI1    RD<-M[EA]                         */
/* 025 019h 000011001b */  0x2031409b,  /* F2RI2    SD<-R[IR2], CAR<-SBR              */
/* 026 01ah 000011010b */  0x0031009c,  /* F2RIS0   EA<-R[IR1]                        */
/* 027 01bh 000011011b */  0x000138bb,  /* F2RIS1   SD<-M[EA]                         */
/* 028 01ch 000011100b */  0x2031409d,  /* F2RIS2   RD<-R[IR2], CAR<-SBR              */
/* 029 01dh 000011101b */  0x00013ebd,  /* F2IM0    RD<-M[PC]                         */
/* 030 01eh 000011110b */  0x000a009f,  /* F2IM1    PC<-PC+1                          */
/* 031 01fh 000011111b */  0x2031409b,  /* F2IM2    SD<-R[IR2], CAR<-SBR              */
/* 032 020h 000100000b */  0x00013ebb,  /* F2IMS0   SD<-M[PC]                         */
/* 033 021h 000100001b */  0x000a009f,  /* F2IMS1   PC<-PC+1                          */
/* 034 022h 000100010b */  0x2031409d,  /* F2IMS2   RD<-R[IR2], CAR<-SBR              */
/* 035 023h 000100011b */  0x00013ebc,  /* F2IN0    EA<-M[PC]                         */
/* 036 024h 000100100b */  0x000a009f,  /* F2IN1    PC<-PC+1                          */
/* 037 025h 000100101b */  0x0000009c,  /* F2IN2    EA<-EA+R[IR1]                     */
/* 038 026h 000100110b */  0x000138bd,  /* F2IN3    RD<-M[EA]                         */
/* 039 027h 000100111b */  0x2031409b,  /* F2IN4    SD<-R[IR2], CAR<-SBR              */
/* 040 028h 000101000b */  0x00013ebc,  /* F2INS0   EA<-M[PC]                         */
/* 041 029h 000101001b */  0x000a009f,  /* F2INS1   PC<-PC+1                          */
/* 042 02ah 000101010b */  0x0000009c,  /* F2INS2   EA<-EA+R[IR1]                     */
/* 043 02bh 000101011b */  0x000138bb,  /* F2INS3   SD<-M[EA]                         */
/* 044 02ch 000101100b */  0x2031409d,  /* F2INS4   RD<-R[IR2], CAR<-SBR              */
/* 045 02dh 000101101b */  0x00313a80,  /* WBR0     R[WBR]<-RD                        */
/* 046 02eh 000101110b */  0x80000200,  /* WBR1     CAR<-IH0                          */
/* 047 02fh 000101111b */  0x83002d00,  /* WBM0     S?CAR<-WBR0 (modo no outro)       */
/* 048 030h 000110000b */  0x00003b1c,  /* WBM1     M[EA]<-RD                         */
/* 049 031h 000110001b */  0x80000200,  /* WBM2     CAR<-IH0                          */
/* 050 032h 000110010b */  0x80000200,  /* NOP0     CAR<-IH0                          */
/* 051 033h 000110011b */  0x804010f8,  /* ENI0     R8<-0010h                         */
/* 052 034h 000110100b */  0x000000d9,  /* ENI1     R9<-RE                            */
/* 053 035h 000110101b */  0x00143298,  /* ENI2     R8<-R8 or R9                      */
/* 054 036h 000110110b */  0x80100218,  /* ENI3     RE<-R8, CAR<-IH0                  */
/* 055 037h 000110111b */  0x80400ff8,  /* DSI0     R8<-000fh                         */
/* 056 038h 000111000b */  0x000000d9,  /* DSI1     R9<-RE                            */
/* 057 039h 000111001b */  0x00123298,  /* DSI2     R8<-R8 and R9                     */
/* 058 03ah 000111010b */  0x80100218,  /* DSI3     RE<-R8, CAR<-IH0                  */
/* 059 03bh 000111011b */  0x00112098,  /* STC0     R8<-not R0                        */
/* 060 03ch 000111100b */  0x010a0018,  /* STC1     R8+1, flag C                      */
/* 061 03dh 000111101b */  0x80000200,  /* STC2     CAR<-IH0                          */
/* 062 03eh 000111110b */  0x01002010,  /* CLC0     R0+R0, flag C                     */
/* 063 03fh 000111111b */  0x80000200,  /* CLC1     CAR<-IH0                          */
/* 064 040h 001000000b */  0x804004f8,  /* CMC0     R8<-0004                          */
/* 065 041h 001000001b */  0x000000d9,  /* CMC1     R9<-RE                            */
/* 066 042h 001000010b */  0x00163298,  /* CMC2     R8<-R8 exor R9                    */
/* 067 043h 001000011b */  0x80100218,  /* CMC3     RE<-R8, CAR<-IH0                  */
/* 068 044h 001000100b */  0x000a009e,  /* RET0     SP<-SP+1                          */
/* 069 045h 001000101b */  0x00013cbf,  /* RET1     PC<-M[SP]                         */
/* 070 046h 001000110b */  0x80000200,  /* RET2     CAR<-IH0                          */
/* 071 047h 001000111b */  0x000a009e,  /* RTI0     SP<-SP+1                          */
/* 072 048h 001001000b */  0x00013cbf,  /* RTI1     PC<-M[SP]                         */
/* 073 049h 001001001b */  0x000a009e,  /* RTI2     SP<-SP+1                          */
/* 074 04ah 001001010b */  0x00013cb8,  /* RTI3     R8<-M[SP]                         */
/* 075 04bh 001001011b */  0x80100218,  /* RTI4     RE<-R8, CAR<-IH0                  */
/* 076 04ch 001001100b */  0x000000d8,  /* INT0     R8<-RE                            */
/* 077 04dh 001001101b */  0x0008319e,  /* INT1     M[SP]<-R8, SP<-SP-1               */
/* 078 04eh 001001110b */  0x00083f9e,  /* INT2     M[SP]<-PC, SP<-SP-1               */
/* 079 04fh 001001111b */  0x8040fff8,  /* INT3     R8<-00ffh                         */
/* 080 050h 001010000b */  0x00128098,  /* INT4     R8<-IR and R8                     */
/* 081 051h 001010001b */  0x804200f9,  /* INT5     R9<-0200h                         */
/* 082 052h 001010010b */  0x00023298,  /* INT6     R8<-R8-R9                         */
/* 083 053h 001010011b */  0x000130bf,  /* INT7     PC<-M[R8]                         */
/* 084 054h 001010100b */  0x80100010,  /* INT8     RE<-R0, CAR<-IF0                  */
/* 085 055h 001010101b */  0x000a009e,  /* RETN0    SP<-SP+1                          */
/* 086 056h 001010110b */  0x00013cbf,  /* RETN1    PC<-M[SP]                         */
/* 087 057h 001010111b */  0x8043fff8,  /* RETN2    R8<-03ffh                         */
/* 088 058h 001011000b */  0x00128098,  /* RETN3    R8<-IR and R8                     */
/* 089 059h 001011001b */  0x0000309e,  /* RETN4    SP<-SP+R8                         */
/* 090 05ah 001011010b */  0x80000200,  /* RETN5    CAR<-IH0                          */
/* 091 05bh 001011011b */  0xe40000f8,  /* NEG0     R8<-0, SBR<-CAR+1, CAR<-F1        */
/* 092 05ch 001011100b */  0x03c23a98,  /* NEG1     R8<-R8-RD, flags ZCNO             */
/* 093 05dh 001011101b */  0x7031309d,  /* NEG2     RD<-R8, CAR<-WB                   */
/* 094 05eh 001011110b */  0xe4000000,  /* INC0     SBR<-CAR+1, CAR<-F1               */
/* 095 05fh 001011111b */  0x73ca009d,  /* INC1     RD<-RD+1, flags ZCNO, CAR<-WB     */
/* 096 060h 001100000b */  0xe4000000,  /* DEC0     SBR<-CAR+1, CAR<-F1               */
/* 097 061h 001100001b */  0x73c8009d,  /* DEC1     RD<-RD-1, flags ZCNO, CAR<-WB     */
/* 098 062h 001100010b */  0xe4000000,  /* COM0     SBR<-CAR+1, CAR<-F1               */
/* 099 063h 001100011b */  0x7290009d,  /* COM1     RD<-!RD, flags ZN, CAR<-WB        */
/* 100 064h 001100100b */  0xe4000000,  /* PUSH0    SBR<-CAR+1, CAR<-F1               */
/* 101 065h 001100101b */  0x00083b9e,  /* PUSH1    M[SP]<-RD, SP<-SP-1               */
/* 102 066h 001100110b */  0x80000200,  /* PUSH2    CAR<-IH0                          */
/* 103 067h 001100111b */  0xe4000000,  /* POP0     SBR<-CAR+1, CAR<-F1               */
/* 104 068h 001101000b */  0x000a009e,  /* POP1     SP<-SP+1                          */
/* 105 069h 001101001b */  0x70013cbd,  /* POP2     RD<-M[SP], CAR<-WB                */
/* 106 06ah 001101010b */  0xe403c0f8,  /* SHR0     R8<-03c0h, SBR<-CAR+1, CAR<-F1    */
/* 107 06bh 001101011b */  0x00128098,  /* SHR1     R8<-R8 and IR                     */
/* 108 06ch 001101100b */  0x804040f9,  /* SHR2     R9<-0040h                         */
/* 109 06dh 001101101b */  0x03a0009d,  /* SHR3     RD<-shr RD, flags ZCN             */
/* 110 06eh 001101110b */  0x00023298,  /* SHR4     R8<-R8-R9                         */
/* 111 06fh 001101111b */  0x80c06d00,  /* SHR5     !z?CAR<-SHR3                      */
/* 112 070h 001110000b */  0x70000000,  /* SHR6     CAR<-WB                           */
/* 113 071h 001110001b */  0xe403c0f8,  /* SHL0     R8<-03c0h, SBR<-CAR+1, CAR<-F1    */
/* 114 072h 001110010b */  0x00128098,  /* SHL1     R8<-R8 and IR                     */
/* 115 073h 001110011b */  0x804040f9,  /* SHL2     R9<-0040h                         */
/* 116 074h 001110100b */  0x03a2009d,  /* SHL3     RD<-shl RD, flags ZCN             */
/* 117 075h 001110101b */  0x00023298,  /* SHL4     R8<-R8-R9                         */
/* 118 076h 001110110b */  0x80c07400,  /* SHL5     !z?CAR<-SHL3                      */
/* 119 077h 001110111b */  0x70000000,  /* SHL6     CAR<-WB                           */
/* 120 078h 001111000b */  0xe403c0f8,  /* SHRA0    R8<-03c0h, SBR<-CAR+1, CAR<-F1    */
/* 121 079h 001111001b */  0x00128098,  /* SHRA1    R8<-R8 and IR                     */
/* 122 07ah 001111010b */  0x804040f9,  /* SHRA2    R9<-0040h                         */
/* 123 07bh 001111011b */  0x03e4009d,  /* SHRA3    RD<-shra RD, flags ZCNO           */
/* 124 07ch 001111100b */  0x00023298,  /* SHRA4    R8<-R8-R9                         */
/* 125 07dh 001111101b */  0x80c07b00,  /* SHRA5    !z?CAR<-SHRA3                     */
/* 126 07eh 001111110b */  0x70000000,  /* SHRA6    CAR<-WB                           */
/* 127 07fh 001111111b */  0xe403c0f8,  /* SHLA0    R8<-03c0h, SBR<-CAR+1, CAR<-F1    */
/* 128 080h 010000000b */  0x00128098,  /* SHLA1    R8<-R8 and IR                     */
/* 129 081h 010000001b */  0x0031209a,  /* SHLA2    R10<-R0                           */
/* 130 082h 010000010b */  0x03e6009d,  /* SHLA3    RD<-shla RD, flags ZCNO           */
/* 131 083h 010000011b */  0x000000d9,  /* SHLA4    R9<-RE                            */
/* 132 084h 010000100b */  0x0014329a,  /* SHLA5    R10<-R10 or R9                    */
/* 133 085h 010000101b */  0x804040f9,  /* SHLA6    R9<-0040h                         */
/* 134 086h 010000110b */  0x00023298,  /* SHLA7    R8<-R8-R9                         */
/* 135 087h 010000111b */  0x80c082d9,  /* SHLA8    R9<-RE, !z?CAR<-SHLA3             */
/* 136 088h 010001000b */  0x804001f8,  /* SHLA9    R8<-1                             */
/* 137 089h 010001001b */  0x0012309a,  /* SHLA10   R10<-R10 and R8                   */
/* 138 08ah 010001010b */  0x0014329a,  /* SHLA11   R10<-R10 or R9                    */
/* 139 08bh 010001011b */  0xf010001a,  /* SHLA12   RE<-R10, CAR<-WB                  */
/* 140 08ch 010001100b */  0xe403c0f8,  /* ROR0     R8<-03c0h, SBR<-CAR+1, CAR<-F1    */
/* 141 08dh 010001101b */  0x00128098,  /* ROR1     R8<-R8 and IR                     */
/* 142 08eh 010001110b */  0x804040f9,  /* ROR2     R9<-0040h                         */
/* 143 08fh 010001111b */  0x03a8009d,  /* ROR3     RD<-ror RD, flags ZCN             */
/* 144 090h 010010000b */  0x00023298,  /* ROR4     R8<-R8-R9                         */
/* 145 091h 010010001b */  0x80c08f00,  /* ROR5     !z?CAR<-ROR3                      */
/* 146 092h 010010010b */  0x70000000,  /* ROR6     CAR<-WB                           */
/* 147 093h 010010011b */  0xe403c0f8,  /* ROL0     R8<-03c0h, SBR<-CAR+1, CAR<-F1    */
/* 148 094h 010010100b */  0x00128098,  /* ROL1     R8<-R8 and IR                     */
/* 149 095h 010010101b */  0x804040f9,  /* ROL2     R9<-0040h                         */
/* 150 096h 010010110b */  0x03aa009d,  /* ROL3     RD<-rol RD, flags ZCN             */
/* 151 097h 010010111b */  0x00023298,  /* ROL4     R8<-R8-R9                         */
/* 152 098h 010011000b */  0x80c09600,  /* ROL5     !z?CAR<-ROL3                      */
/* 153 099h 010011001b */  0x70000000,  /* ROL6     CAR<-WB                           */
/* 154 09ah 010011010b */  0xe403c0f8,  /* RORC0    R8<-03c0h, SBR<-CAR+1, CAR<-F1    */
/* 155 09bh 010011011b */  0x00128098,  /* RORC1    R8<-R8 and IR                     */
/* 156 09ch 010011100b */  0x804040f9,  /* RORC2    R9<-0040h                         */
/* 157 09dh 010011101b */  0x03ac009d,  /* RORC3    RD<-rorc RD, flags ZCN            */
/* 158 09eh 010011110b */  0x00023298,  /* RORC4    R8<-R8-R9                         */
/* 159 09fh 010011111b */  0x80c09d00,  /* RORC5    !z?CAR<-RORC3                     */
/* 160 0a0h 010100000b */  0x70000000,  /* RORC6    CAR<-WB                           */
/* 161 0a1h 010100001b */  0xe403c0f8,  /* ROLC0    R8<-03c0h, SBR<-CAR+1, CAR<-F1    */
/* 162 0a2h 010100010b */  0x00128098,  /* ROLC1    R8<-R8 and IR                     */
/* 163 0a3h 010100011b */  0x804040f9,  /* ROLC2    R9<-0040h                         */
/* 164 0a4h 010100100b */  0x03ae009d,  /* ROLC3    RD<-rolc RD, flags ZCN            */
/* 165 0a5h 010100101b */  0x00023298,  /* ROLC4    R8<-R8-R9                         */
/* 166 0a6h 010100110b */  0x80c0a400,  /* ROLC5    !z?CAR<-ROLC3                     */
/* 167 0a7h 010100111b */  0x70000000,  /* ROLC6    CAR<-WB                           */
/* 168 0a8h 010101000b */  0xec000000,  /* MOV0     SBR<-CAR+1, CAR<-F2               */
/* 169 0a9h 010101001b */  0x7031369d,  /* MOV1     RD<-SD, CAR<-WB                   */
/* 170 0aah 010101010b */  0xec00fff8,  /* MVBL0    R8<-00ffh, SBR<-CAR+1, CAR<-F2    */
/* 171 0abh 010101011b */  0x00113099,  /* MVBL1    R9<-!R8                           */
/* 172 0ach 010101100b */  0x0012329d,  /* MVBL2    RD<-RD and R9                     */
/* 173 0adh 010101101b */  0x00123698,  /* MVBL3    R8<-R8 and SD                     */
/* 174 0aeh 010101110b */  0x7014309d,  /* MVBL4    RD<-RD or R8, CAR<-WB             */
/* 175 0afh 010101111b */  0xec00fff8,  /* MVBH0    R8<-00ffh, SBR<-CAR+1, CAR<-F2    */
/* 176 0b0h 010110000b */  0x00113099,  /* MVBH1    R9<-!R8                           */
/* 177 0b1h 010110001b */  0x0012309d,  /* MVBH2    RD<-RD and R8                     */
/* 178 0b2h 010110010b */  0x00123699,  /* MVBH3    R9<-R9 and SD                     */
/* 179 0b3h 010110011b */  0x7014329d,  /* MVBH4    RD<-RD or R9, CAR<-WB             */
/* 180 0b4h 010110100b */  0xec000000,  /* ADD0     SBR<-CAR+1, CAR<-F2               */
/* 181 0b5h 010110101b */  0x73c0369d,  /* ADD1     RD<-RD+SD, flags ZCNO, CAR<-WB    */
/* 182 0b6h 010110110b */  0xec000000,  /* ADDC0    SBR<-CAR+1, CAR<-F2               */
/* 183 0b7h 010110111b */  0x73c4369d,  /* ADDC1    RD<-RD+SD+C, flags ZCNO, CAR<-WB  */
/* 184 0b8h 010111000b */  0xec000000,  /* SUB0     SBR<-CAR+1, CAR<-F2               */
/* 185 0b9h 010111001b */  0x73c2369d,  /* SUB1     RD<-RD-SD, flags ZCNO, CAR<-WB    */
/* 186 0bah 010111010b */  0xec000000,  /* SUBB0    SBR<-CAR+1, CAR<-F2               */
/* 187 0bbh 010111011b */  0x73c6369d,  /* SUBB1    RD<-RD-SD-C, flags ZCNO, CAR<-WB  */
/* 188 0bch 010111100b */  0xec000000,  /* AND0     SBR<-CAR+1, CAR<-F2               */
/* 189 0bdh 010111101b */  0x7292369d,  /* AND1     RD<-RD and SD, flags ZN, CAR<-WB  */
/* 190 0beh 010111110b */  0xec000000,  /* OR0      SBR<-CAR+1, CAR<-F2               */
/* 191 0bfh 010111111b */  0x7294369d,  /* OR1      RD<-RD or SD, flags ZN, CAR<-WB   */
/* 192 0c0h 011000000b */  0xec000000,  /* XOR0     SBR<-CAR+1, CAR<-F2               */
/* 193 0c1h 011000001b */  0x7296369d,  /* XOR1     RD<-RD xor SD, flags ZN, CAR<-WB  */
/* 194 0c2h 011000010b */  0xec000000,  /* CMP0     SBR<-CAR+1, CAR<-F2               */
/* 195 0c3h 011000011b */  0x73c2361d,  /* CMP1     RD<-RD-SD, flags ZCNO, CAR<-WB    */
/* 196 0c4h 011000100b */  0xec000000,  /* TEST0    SBR<-CAR+1, CAR<-F2               */
/* 197 0c5h 011000101b */  0x7292361d,  /* TEST1    RD<-RD and SD, flags ZN, CAR<-WB  */
/* 198 0c6h 011000110b */  0x8340c900,  /* WSD0     !S?CAR<-WSD3       (mode on RD)   */
/* 199 0c7h 011000111b */  0x8240c900,  /* WSD1     !M0?CAR<-WSD3 (mode REG or IMM)   */
/* 200 0c8h 011001000b */  0x7000371c,  /* WSD2     M[EA]<-SD, CAR<-WB   (mode MEM)   */
/* 201 0c9h 011001001b */  0x70317680,  /* WSD3     R[!WBR]<-SD, CAR<-WB (mode REG)   */
/* 202 0cah 011001010b */  0xec000000,  /* XCH0     SBR<-CAR+1, CAR<-F2               */
/* 203 0cbh 011001011b */  0x00313a98,  /* XCH1     R8<-RD                            */
/* 204 0cch 011001100b */  0x0031369d,  /* XCH2     RD<-SD                            */
/* 205 0cdh 011001101b */  0x0031309b,  /* XCH3     SD<-R8                            */
/* 206 0ceh 011001110b */  0x8000c600,  /* XCH4     CAR<-WSD0                         */
/* 207 0cfh 011001111b */  0xec0010f8,  /* MUL0     R8<-16, SBR<-CAR+1, CAR<-F2       */
/* 208 0d0h 011010000b */  0x000000da,  /* MUL1     R10<-RE                           */
/* 209 0d1h 011010001b */  0x0012309a,  /* MUL2     R10<-R10 and R8   (flag E)        */
/* 210 0d2h 011010010b */  0x00313a99,  /* MUL3     R9<-RD                            */
/* 211 0d3h 011010011b */  0x01f1209d,  /* MUL4     RD<-R0, flags CNO (clear flags)   */
/* 212 0d4h 011010100b */  0x002c009b,  /* MUL5     SD<-rorc SD                       */
/* 213 0d5h 011010101b */  0x8150d71a,  /* MUL6     RE<-R10, !c?CAR<-MUL8             */
/* 214 0d6h 011010110b */  0x0100329d,  /* MUL7     RD<-RD+R9, flag C                 */
/* 215 0d7h 011010111b */  0x012c009d,  /* MUL8     RD<-rorc RD, flag C               */
/* 216 0d8h 011011000b */  0x00080098,  /* MUL9     R8<-R8-1                          */
/* 217 0d9h 011011001b */  0x80c0d400,  /* MUL10    !z?CAR<-MUL5                      */
/* 218 0dah 011011010b */  0x012c009b,  /* MUL11    SD<-rorc SD, flag C (C=0)         */
/* 219 0dbh 011011011b */  0x0200361d,  /* MUL12    RD+SD, flag Z                     */
/* 220 0dch 011011100b */  0x8000c600,  /* MUL13    CAR<-WSD0                         */
/* 221 0ddh 011011101b */  0xec0000d8,  /* DIV0     R8<-RE, SBR<-CAR+1, CAR<-F2       */
/* 222 0deh 011011110b */  0x0000201b,  /* DIV1     SD<-SD+R0                         */
/* 223 0dfh 011011111b */  0x80c0e300,  /* DIV2     !z?CAR<-DIV6                      */
/* 224 0e0h 011100000b */  0x804001f9,  /* DIV3     R9<-0001         (divisao por 0!) */
/* 225 0e1h 011100001b */  0x00143298,  /* DIV4     R8<-R8 or R9                      */
/* 226 0e2h 011100010b */  0x80100218,  /* DIV5     RE<-R8, CAR<-IH0           (O<-1) */
/* 227 0e3h 011100011b */  0x01c12099,  /* DIV6     R9<-R0+R0, flags CNO (clear flag) */
/* 228 0e4h 011100100b */  0x0002361d,  /* DIV7     RD-SD                             */
/* 229 0e5h 011100101b */  0x8140f500,  /* DIV8     !c?CAR<-DIV24          (result=0) */
/* 230 0e6h 011100110b */  0x00312098,  /* DIV9     R8<-R0                            */
/* 231 0e7h 011100111b */  0x000a0098,  /* DIV10    R8<-R8+1                          */
/* 232 0e8h 011101000b */  0x0122009b,  /* DIV11    SD<-shl SD, flag C                */
/* 233 0e9h 011101001b */  0x8100ec00,  /* DIV12    c?CAR<-DIV15                      */
/* 234 0eah 011101010b */  0x0002361d,  /* DIV13    RD-SD                             */
/* 235 0ebh 011101011b */  0x8100e700,  /* DIV14    c?CAR<-DIV10                      */
/* 236 0ech 011101100b */  0x002c009b,  /* DIV15    SD<-rorc SD                       */
/* 237 0edh 011101101b */  0x0102369d,  /* DIV16    RD<-RD-SD, flag C                 */
/* 238 0eeh 011101110b */  0x8100f100,  /* DIV17    c?CAR<-DIV20                      */
/* 239 0efh 011101111b */  0x0000369d,  /* DIV18    RD<-RD+SD              (<0:repoe) */
/* 240 0f0h 011110000b */  0x01300010,  /* DIV19    R0, flag C             (C<-0)     */
/* 241 0f1h 011110001b */  0x002e0099,  /* DIV20    R9<-rolc R9                       */
/* 242 0f2h 011110010b */  0x0020009b,  /* DIV21    SD<-shr SD                        */
/* 243 0f3h 011110011b */  0x00080098,  /* DIV22    R8<-R8-1                          */
/* 244 0f4h 011110100b */  0x80c0ed00,  /* DIV23    !z?CAR<-DIV16                     */
/* 245 0f5h 011110101b */  0x00313a9b,  /* DIV24    SD<-RD                            */
/* 246 0f6h 011110110b */  0x0331329d,  /* DIV25    RD<-R9, flags ZC                  */
/* 247 0f7h 011110111b */  0x8000c600,  /* DIV26    CAR<-WSD0                         */
/* 248 0f8h 011111000b */  0x83c00200,  /* BR.C0    !COND?CAR<-IH0                    */
/* 249 0f9h 011111001b */  0x80403ff8,  /* BR0      R8<-003fh                         */
/* 250 0fah 011111010b */  0x0013b099,  /* BR1      R9<-R8 and RI                     */
/* 251 0fbh 011111011b */  0x804020fa,  /* BR2      R10<-0020h       (teste do sinal) */
/* 252 0fch 011111100b */  0x0012329a,  /* BR3      R10<-R10 and R9                   */
/* 253 0fdh 011111101b */  0x80810000,  /* BR4      z?CAR<-BR7                        */
/* 254 0feh 011111110b */  0x00100098,  /* BR5      R8<-not R8                        */
/* 255 0ffh 011111111b */  0x00143099,  /* BR6      R9<-R9 or R8                      */
/* 256 100h 100000000b */  0x0000329f,  /* BR7      PC<-PC+R9                         */
/* 257 101h 100000001b */  0x80000200,  /* BR8      CAR<-IH0                          */
/* 258 102h 100000010b */  0xe4000000,  /* JMP0     SBR<-CAR+1, CAR<-F1               */
/* 259 103h 100000011b */  0x00313a9f,  /* JMP1     PC<-RD                            */
/* 260 104h 100000100b */  0x80000200,  /* JMP2     CAR<-IH0                          */
/* 261 105h 100000101b */  0xe4000000,  /* JMP.C0   SBR<-CAR+1, CAR<-F1               */
/* 262 106h 100000110b */  0x83c00200,  /* JMP.C1   !COND?CAR<-IH0                    */
/* 263 107h 100000111b */  0x00313a9f,  /* JMP.C2   PC<-RD                            */
/* 264 108h 100001000b */  0x80000200,  /* JMP.C3   CAR<-IH0                          */
/* 265 109h 100001001b */  0xe4000000,  /* CALL0    SBR<-CAR+1, CAR<-F1               */
/* 266 10ah 100001010b */  0x00083f9e,  /* CALL1    M[SP]<-PC, SP<-SP-1               */
/* 267 10bh 100001011b */  0x00313a9f,  /* CALL2    PC<-RD                            */
/* 268 10ch 100001100b */  0x80000200,  /* CALL3    CAR<-IH0                          */
/* 269 10dh 100001101b */  0xe4000000,  /* CALL.C0  SBR<-CAR+1, CAR<-F1               */
/* 270 10eh 100001110b */  0x83c00200,  /* CALL.C1  !COND?CAR<-IH0                    */
/* 271 10fh 100001111b */  0x00083f9e,  /* CALL.C2  M[SP]<-PC, SP<-SP-1               */
/* 272 110h 100010000b */  0x00313a9f,  /* CALL.C3  PC<-RD                            */
/* 273 111h 100010001b */  0x80000200,  /* CALL.C4  CAR<-IH0                          */
/* 274 112h 100010010b */  0x0,  /* Unused */
/* 275 113h 100010011b */  0x0,  /* Unused */
/* 276 114h 100010100b */  0x0,  /* Unused */
/* 277 115h 100010101b */  0x0,  /* Unused */
/* 278 116h 100010110b */  0x0,  /* Unused */
/* 279 117h 100010111b */  0x0,  /* Unused */
/* 280 118h 100011000b */  0x0,  /* Unused */
/* 281 119h 100011001b */  0x0,  /* Unused */
/* 282 11ah 100011010b */  0x0,  /* Unused */
/* 283 11bh 100011011b */  0x0,  /* Unused */
/* 284 11ch 100011100b */  0x0,  /* Unused */
/* 285 11dh 100011101b */  0x0,  /* Unused */
/* 286 11eh 100011110b */  0x0,  /* Unused */
/* 287 11fh 100011111b */  0x0,  /* Unused */
/* 288 120h 100100000b */  0x0,  /* Unused */
/* 289 121h 100100001b */  0x0,  /* Unused */
/* 290 122h 100100010b */  0x0,  /* Unused */
/* 291 123h 100100011b */  0x0,  /* Unused */
/* 292 124h 100100100b */  0x0,  /* Unused */
/* 293 125h 100100101b */  0x0,  /* Unused */
/* 294 126h 100100110b */  0x0,  /* Unused */
/* 295 127h 100100111b */  0x0,  /* Unused */
/* 296 128h 100101000b */  0x0,  /* Unused */
/* 297 129h 100101001b */  0x0,  /* Unused */
/* 298 12ah 100101010b */  0x0,  /* Unused */
/* 299 12bh 100101011b */  0x0,  /* Unused */
/* 300 12ch 100101100b */  0x0,  /* Unused */
/* 301 12dh 100101101b */  0x0,  /* Unused */
/* 302 12eh 100101110b */  0x0,  /* Unused */
/* 303 12fh 100101111b */  0x0,  /* Unused */
/* 304 130h 100110000b */  0x0,  /* Unused */
/* 305 131h 100110001b */  0x0,  /* Unused */
/* 306 132h 100110010b */  0x0,  /* Unused */
/* 307 133h 100110011b */  0x0,  /* Unused */
/* 308 134h 100110100b */  0x0,  /* Unused */
/* 309 135h 100110101b */  0x0,  /* Unused */
/* 310 136h 100110110b */  0x0,  /* Unused */
/* 311 137h 100110111b */  0x0,  /* Unused */
/* 312 138h 100111000b */  0x0,  /* Unused */
/* 313 139h 100111001b */  0x0,  /* Unused */
/* 314 13ah 100111010b */  0x0,  /* Unused */
/* 315 13bh 100111011b */  0x0,  /* Unused */
/* 316 13ch 100111100b */  0x0,  /* Unused */
/* 317 13dh 100111101b */  0x0,  /* Unused */
/* 318 13eh 100111110b */  0x0,  /* Unused */
/* 319 13fh 100111111b */  0x0,  /* Unused */
/* 320 140h 101000000b */  0x0,  /* Unused */
/* 321 141h 101000001b */  0x0,  /* Unused */
/* 322 142h 101000010b */  0x0,  /* Unused */
/* 323 143h 101000011b */  0x0,  /* Unused */
/* 324 144h 101000100b */  0x0,  /* Unused */
/* 325 145h 101000101b */  0x0,  /* Unused */
/* 326 146h 101000110b */  0x0,  /* Unused */
/* 327 147h 101000111b */  0x0,  /* Unused */
/* 328 148h 101001000b */  0x0,  /* Unused */
/* 329 149h 101001001b */  0x0,  /* Unused */
/* 330 14ah 101001010b */  0x0,  /* Unused */
/* 331 14bh 101001011b */  0x0,  /* Unused */
/* 332 14ch 101001100b */  0x0,  /* Unused */
/* 333 14dh 101001101b */  0x0,  /* Unused */
/* 334 14eh 101001110b */  0x0,  /* Unused */
/* 335 14fh 101001111b */  0x0,  /* Unused */
/* 336 150h 101010000b */  0x0,  /* Unused */
/* 337 151h 101010001b */  0x0,  /* Unused */
/* 338 152h 101010010b */  0x0,  /* Unused */
/* 339 153h 101010011b */  0x0,  /* Unused */
/* 340 154h 101010100b */  0x0,  /* Unused */
/* 341 155h 101010101b */  0x0,  /* Unused */
/* 342 156h 101010110b */  0x0,  /* Unused */
/* 343 157h 101010111b */  0x0,  /* Unused */
/* 344 158h 101011000b */  0x0,  /* Unused */
/* 345 159h 101011001b */  0x0,  /* Unused */
/* 346 15ah 101011010b */  0x0,  /* Unused */
/* 347 15bh 101011011b */  0x0,  /* Unused */
/* 348 15ch 101011100b */  0x0,  /* Unused */
/* 349 15dh 101011101b */  0x0,  /* Unused */
/* 350 15eh 101011110b */  0x0,  /* Unused */
/* 351 15fh 101011111b */  0x0,  /* Unused */
/* 352 160h 101100000b */  0x0,  /* Unused */
/* 353 161h 101100001b */  0x0,  /* Unused */
/* 354 162h 101100010b */  0x0,  /* Unused */
/* 355 163h 101100011b */  0x0,  /* Unused */
/* 356 164h 101100100b */  0x0,  /* Unused */
/* 357 165h 101100101b */  0x0,  /* Unused */
/* 358 166h 101100110b */  0x0,  /* Unused */
/* 359 167h 101100111b */  0x0,  /* Unused */
/* 360 168h 101101000b */  0x0,  /* Unused */
/* 361 169h 101101001b */  0x0,  /* Unused */
/* 362 16ah 101101010b */  0x0,  /* Unused */
/* 363 16bh 101101011b */  0x0,  /* Unused */
/* 364 16ch 101101100b */  0x0,  /* Unused */
/* 365 16dh 101101101b */  0x0,  /* Unused */
/* 366 16eh 101101110b */  0x0,  /* Unused */
/* 367 16fh 101101111b */  0x0,  /* Unused */
/* 368 170h 101110000b */  0x0,  /* Unused */
/* 369 171h 101110001b */  0x0,  /* Unused */
/* 370 172h 101110010b */  0x0,  /* Unused */
/* 371 173h 101110011b */  0x0,  /* Unused */
/* 372 174h 101110100b */  0x0,  /* Unused */
/* 373 175h 101110101b */  0x0,  /* Unused */
/* 374 176h 101110110b */  0x0,  /* Unused */
/* 375 177h 101110111b */  0x0,  /* Unused */
/* 376 178h 101111000b */  0x0,  /* Unused */
/* 377 179h 101111001b */  0x0,  /* Unused */
/* 378 17ah 101111010b */  0x0,  /* Unused */
/* 379 17bh 101111011b */  0x0,  /* Unused */
/* 380 17ch 101111100b */  0x0,  /* Unused */
/* 381 17dh 101111101b */  0x0,  /* Unused */
/* 382 17eh 101111110b */  0x0,  /* Unused */
/* 383 17fh 101111111b */  0x0,  /* Unused */
/* 384 180h 110000000b */  0x0,  /* Unused */
/* 385 181h 110000001b */  0x0,  /* Unused */
/* 386 182h 110000010b */  0x0,  /* Unused */
/* 387 183h 110000011b */  0x0,  /* Unused */
/* 388 184h 110000100b */  0x0,  /* Unused */
/* 389 185h 110000101b */  0x0,  /* Unused */
/* 390 186h 110000110b */  0x0,  /* Unused */
/* 391 187h 110000111b */  0x0,  /* Unused */
/* 392 188h 110001000b */  0x0,  /* Unused */
/* 393 189h 110001001b */  0x0,  /* Unused */
/* 394 18ah 110001010b */  0x0,  /* Unused */
/* 395 18bh 110001011b */  0x0,  /* Unused */
/* 396 18ch 110001100b */  0x0,  /* Unused */
/* 397 18dh 110001101b */  0x0,  /* Unused */
/* 398 18eh 110001110b */  0x0,  /* Unused */
/* 399 18fh 110001111b */  0x0,  /* Unused */
/* 400 190h 110010000b */  0x0,  /* Unused */
/* 401 191h 110010001b */  0x0,  /* Unused */
/* 402 192h 110010010b */  0x0,  /* Unused */
/* 403 193h 110010011b */  0x0,  /* Unused */
/* 404 194h 110010100b */  0x0,  /* Unused */
/* 405 195h 110010101b */  0x0,  /* Unused */
/* 406 196h 110010110b */  0x0,  /* Unused */
/* 407 197h 110010111b */  0x0,  /* Unused */
/* 408 198h 110011000b */  0x0,  /* Unused */
/* 409 199h 110011001b */  0x0,  /* Unused */
/* 410 19ah 110011010b */  0x0,  /* Unused */
/* 411 19bh 110011011b */  0x0,  /* Unused */
/* 412 19ch 110011100b */  0x0,  /* Unused */
/* 413 19dh 110011101b */  0x0,  /* Unused */
/* 414 19eh 110011110b */  0x0,  /* Unused */
/* 415 19fh 110011111b */  0x0,  /* Unused */
/* 416 1a0h 110100000b */  0x0,  /* Unused */
/* 417 1a1h 110100001b */  0x0,  /* Unused */
/* 418 1a2h 110100010b */  0x0,  /* Unused */
/* 419 1a3h 110100011b */  0x0,  /* Unused */
/* 420 1a4h 110100100b */  0x0,  /* Unused */
/* 421 1a5h 110100101b */  0x0,  /* Unused */
/* 422 1a6h 110100110b */  0x0,  /* Unused */
/* 423 1a7h 110100111b */  0x0,  /* Unused */
/* 424 1a8h 110101000b */  0x0,  /* Unused */
/* 425 1a9h 110101001b */  0x0,  /* Unused */
/* 426 1aah 110101010b */  0x0,  /* Unused */
/* 427 1abh 110101011b */  0x0,  /* Unused */
/* 428 1ach 110101100b */  0x0,  /* Unused */
/* 429 1adh 110101101b */  0x0,  /* Unused */
/* 430 1aeh 110101110b */  0x0,  /* Unused */
/* 431 1afh 110101111b */  0x0,  /* Unused */
/* 432 1b0h 110110000b */  0x0,  /* Unused */
/* 433 1b1h 110110001b */  0x0,  /* Unused */
/* 434 1b2h 110110010b */  0x0,  /* Unused */
/* 435 1b3h 110110011b */  0x0,  /* Unused */
/* 436 1b4h 110110100b */  0x0,  /* Unused */
/* 437 1b5h 110110101b */  0x0,  /* Unused */
/* 438 1b6h 110110110b */  0x0,  /* Unused */
/* 439 1b7h 110110111b */  0x0,  /* Unused */
/* 440 1b8h 110111000b */  0x0,  /* Unused */
/* 441 1b9h 110111001b */  0x0,  /* Unused */
/* 442 1bah 110111010b */  0x0,  /* Unused */
/* 443 1bbh 110111011b */  0x0,  /* Unused */
/* 444 1bch 110111100b */  0x0,  /* Unused */
/* 445 1bdh 110111101b */  0x0,  /* Unused */
/* 446 1beh 110111110b */  0x0,  /* Unused */
/* 447 1bfh 110111111b */  0x0,  /* Unused */
/* 448 1c0h 111000000b */  0x0,  /* Unused */
/* 449 1c1h 111000001b */  0x0,  /* Unused */
/* 450 1c2h 111000010b */  0x0,  /* Unused */
/* 451 1c3h 111000011b */  0x0,  /* Unused */
/* 452 1c4h 111000100b */  0x0,  /* Unused */
/* 453 1c5h 111000101b */  0x0,  /* Unused */
/* 454 1c6h 111000110b */  0x0,  /* Unused */
/* 455 1c7h 111000111b */  0x0,  /* Unused */
/* 456 1c8h 111001000b */  0x0,  /* Unused */
/* 457 1c9h 111001001b */  0x0,  /* Unused */
/* 458 1cah 111001010b */  0x0,  /* Unused */
/* 459 1cbh 111001011b */  0x0,  /* Unused */
/* 460 1cch 111001100b */  0x0,  /* Unused */
/* 461 1cdh 111001101b */  0x0,  /* Unused */
/* 462 1ceh 111001110b */  0x0,  /* Unused */
/* 463 1cfh 111001111b */  0x0,  /* Unused */
/* 464 1d0h 111010000b */  0x0,  /* Unused */
/* 465 1d1h 111010001b */  0x0,  /* Unused */
/* 466 1d2h 111010010b */  0x0,  /* Unused */
/* 467 1d3h 111010011b */  0x0,  /* Unused */
/* 468 1d4h 111010100b */  0x0,  /* Unused */
/* 469 1d5h 111010101b */  0x0,  /* Unused */
/* 470 1d6h 111010110b */  0x0,  /* Unused */
/* 471 1d7h 111010111b */  0x0,  /* Unused */
/* 472 1d8h 111011000b */  0x0,  /* Unused */
/* 473 1d9h 111011001b */  0x0,  /* Unused */
/* 474 1dah 111011010b */  0x0,  /* Unused */
/* 475 1dbh 111011011b */  0x0,  /* Unused */
/* 476 1dch 111011100b */  0x0,  /* Unused */
/* 477 1ddh 111011101b */  0x0,  /* Unused */
/* 478 1deh 111011110b */  0x0,  /* Unused */
/* 479 1dfh 111011111b */  0x0,  /* Unused */
/* 480 1e0h 111100000b */  0x0,  /* Unused */
/* 481 1e1h 111100001b */  0x0,  /* Unused */
/* 482 1e2h 111100010b */  0x0,  /* Unused */
/* 483 1e3h 111100011b */  0x0,  /* Unused */
/* 484 1e4h 111100100b */  0x0,  /* Unused */
/* 485 1e5h 111100101b */  0x0,  /* Unused */
/* 486 1e6h 111100110b */  0x0,  /* Unused */
/* 487 1e7h 111100111b */  0x0,  /* Unused */
/* 488 1e8h 111101000b */  0x0,  /* Unused */
/* 489 1e9h 111101001b */  0x0,  /* Unused */
/* 490 1eah 111101010b */  0x0,  /* Unused */
/* 491 1ebh 111101011b */  0x0,  /* Unused */
/* 492 1ech 111101100b */  0x0,  /* Unused */
/* 493 1edh 111101101b */  0x0,  /* Unused */
/* 494 1eeh 111101110b */  0x0,  /* Unused */
/* 495 1efh 111101111b */  0x0,  /* Unused */
/* 496 1f0h 111110000b */  0x0,  /* Unused */
/* 497 1f1h 111110001b */  0x0,  /* Unused */
/* 498 1f2h 111110010b */  0x0,  /* Unused */
/* 499 1f3h 111110011b */  0x0,  /* Unused */
/* 500 1f4h 111110100b */  0x0,  /* Unused */
/* 501 1f5h 111110101b */  0x0,  /* Unused */
/* 502 1f6h 111110110b */  0x0,  /* Unused */
/* 503 1f7h 111110111b */  0x0,  /* Unused */
/* 504 1f8h 111111000b */  0x0,  /* Unused */
/* 505 1f9h 111111001b */  0x0,  /* Unused */
/* 506 1fah 111111010b */  0x0,  /* Unused */
/* 507 1fbh 111111011b */  0x0,  /* Unused */
/* 508 1fch 111111100b */  0x0,  /* Unused */
/* 509 1fdh 111111101b */  0x0,  /* Unused */
/* 510 1feh 111111110b */  0x0,  /* Unused */
/* 511 1ffh 111111111b */  0x0   /* Unused */
};


/*-----------------------------------------------------------------------*/
dword read_control_ROM(ctrlROM_addr ROMaddress)
{
    return controlROM[ROMaddress];
}

/*-----------------------------------------------------------------------*/
void write_control_ROM(ctrlROM_addr ROMaddress, dword uinst)
{
    controlROM[ROMaddress] = uinst;
}
