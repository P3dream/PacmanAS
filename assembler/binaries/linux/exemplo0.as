;------------------------------------------------------------------------------
; ZONA I: Definicao de constantes
;         Pseudo-instrucao : EQU
;------------------------------------------------------------------------------
CR                EQU     0Ah
FIM_TEXTO         EQU     '@'
FIM_MAPA          EQU     99d
IO_READ           EQU     FFFFh
IO_WRITE          EQU     FFFEh
IO_STATUS         EQU     FFFDh
INITIAL_SP        EQU     FDFFh
CURSOR		EQU     FFFCh
CURSOR_INIT		EQU	  FFFFh
ROW_POSITION	EQU	  0d
COL_POSITION	EQU	  0d
ROW_SHIFT		EQU		8d
COLUMN_SHIF0T     EQU		8d    
PACMAN_X          EQU         1d
PACMAN_Y          EQU         1d
UP                EQU         0d
DOWN              EQU         1d
LEFT              EQU         2d
RIGHT             EQU     3d
TIMER_UNITS       EQU     FFF6h
ACTIVATE_TIMER    EQU     FFF7h
ON                EQU     1d
OFF               EQU     0d

;------------------------------------------------------------------------------
; ZONA II: definicao de variaveis
;          Pseudo-instrucoes : WORD - palavra (16 bits)
;                              STR  - sequencia de caracteres (cada ocupa 1 palavra: 16 bits).
;          Cada caracter ocupa 1 palavra
;------------------------------------------------------------------------------

                ORIG    8000h
L1  		    STR     '################################################################################', FIM_TEXTO
L2              STR     '#C     .#           #                  #    ## . .    #       #.       #       #', FIM_TEXTO
L3              STR     '#     . #        .....  .....  ..... #     ## .    .  #       #.       #       #', FIM_TEXTO
L4              STR     '#      .#        .      .      .      #      ###    . #       #.       #....   #', FIM_TEXTO
L5              STR     '#   .    .#     ##.....  ...... ..... ####             #  ...  ####     #####  #', FIM_TEXTO
L6              STR     '#      .#     #  .   .  .    . .   .                .    .       #             #', FIM_TEXTO
L7              STR     '#      .#     #  .....  ...... .....                             #             #', FIM_TEXTO
L8              STR     '#.      #######       ###############  ################          #####         #', FIM_TEXTO
L9              STR     '#.                    #                               #  ####         .....    #', FIM_TEXTO
L10             STR     '#.                    #                               #     #                  #', FIM_TEXTO
L11             STR     '#.    ####            #                               #     #        #         #', FIM_TEXTO
L12             STR     '#.       ##### ...                                             .       #########', FIM_TEXTO
L13             STR     '#.                    #                               ##        .              #', FIM_TEXTO
L14             STR     '#.     ###       ######                               ##       #####           #', FIM_TEXTO
L15             STR     '#.                    #                               #      .  #..#    #      #', FIM_TEXTO
L16             STR     '#      #######       ################ ################         ..#.. .. #     .#', FIM_TEXTO
L17             STR     '#         #                #                                   ..########     .#', FIM_TEXTO 
L18             STR     '#         #                #                #               #                 .#', FIM_TEXTO
L19             STR     '#         ##.....  .....  . .    ....   .......#########    #   ##      #      #', FIM_TEXTO
L20             STR     '#         ##.   .  .      .   .  .   .  .## ##.         #   #####  ..      #####', FIM_TEXTO
L21             STR     '#         ##.   .  .....  .    . . .    .#   #.     ..    ..    #     ...     .#', FIM_TEXTO
L22             STR     '#         ##.....  .####  .   .  . ..   .## ##.  #   ..   .        #.    .     #', FIM_TEXTO
L23             STR     '#         ##.      .....  ....   .   .  .......  #    .. .. #      #  .    .   #', FIM_TEXTO
L24             STR     '################################################################################', FIM_MAPA

RowIndex		WORD	0d
ColumnIndex		WORD	0d
TextIndex		WORD	0d
EnderecoString    WORD  0d
PacmanDirection   WORD  RIGHT

;------------------------------------------------------------------------------
; ZONA III: definicao de tabela de interrupções
;------------------------------------------------------------------------------
                ORIG    FE00h
INT0            WORD    MoverDireita
                ORIG    FE0Fh

INT15           WORD    Timer

;------------------------------------------------------------------------------
; ZONA IV: codigo
;        conjunto de instrucoes Assembly, ordenadas de forma a realizar
;        as funcoes pretendidas
;------------------------------------------------------------------------------
                ORIG    0000h
                JMP     Main

Timer:            PUSH R1
                  PUSH R2

                  MOV R1,PACMAN_X
                  MOV R2,PACMAN_Y
                  
                  SHL R1,8d
                  OR R1,R2
                  MOV M[CURSOR], R1
                  MOV R1,'C'
                  MOV M[IO_WRITE],R1

                  POP R2
                  POP R1
                  RTI

MoverDireita:     PUSH R1
                  PUSH R2

                  MOV R1, RIGHT
                  MOV M[PacmanDirection], R1


                  POP R2
                  POP R1


                  RTI

;------------------------------------------------------------------------------
; Função printar string
;------------------------------------------------------------------------------
PrintarMapa:PUSH    R1
            PUSH    R2
            PUSH    R3

            MOV     R1, L1
	      MOV M[ TextIndex ], R1

            While:MOV R1, M[ TextIndex ]
                  MOV   R2, M[ RowIndex ]
                  MOV   R3, M[ ColumnIndex ]
                  SHL   R2, 8d
                  MOV	R1, M[ R1 ]
                  OR    R2, R3
                  MOV   M[ CURSOR ], R2
                                
                  CMP   R1, FIM_TEXTO
                  JMP.Z EndWhile
                  CMP   R1, FIM_MAPA
                  JMP.Z FimPrintMapa
                  MOV   M[ IO_WRITE ], R1 
                                
                  INC	M[ ColumnIndex ]
                  INC	M[ TextIndex ]

                  JMP   While 

           EndWhile:MOV     R1, 0d
                    INC     M[ RowIndex ]
                    INC     M[ TextIndex ]
                    MOV     M[ ColumnIndex ], R1 ;Zera coluna
                    JMP     While

FimPrintMapa:   MOV     R1, 99d 
                MOV     M[ ColumnIndex ], R1
                POP     R3
                POP     R2
                POP     R1
                RET

Configura_Timer:  PUSH R1
                  PUSH R2

                  MOV R1,10d
                  MOV M[TIMER_UNITS], R1
                  MOV R1, ON
                  MOV M[ACTIVATE_TIMER], R1

                  POP R2
                  POP R1
                  RET

VerificaMapa:     PUSH R1
                  PUSH R2

                  MOV R1, PacmanDirection
                  CMP R1,RIGHT
                  

                  POP R2
                  POP R1

;------------------------------------------------------------------------------
; Função Main
;------------------------------------------------------------------------------
Main:			ENI

				MOV		R1, INITIAL_SP
				MOV		SP, R1		 		; We need to initialize the stack
				MOV		R1, CURSOR_INIT		; We need to initialize the cursor 
				MOV		M[ CURSOR ], R1		; with value CURSOR_INIT

				CALL PrintarMapa

                        MOV R1,10d
                        MOV M[TIMER_UNITS],R1
                        MOV R1,ON
                        MOV M[ACTIVATE_TIMER],R1



Cycle: 			BR		Cycle	
Halt:           BR		Halt