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
CURSOR		    EQU     FFFCh
CURSOR_INIT	    EQU     FFFFh
ROW_POSITION	    EQU     0d
COL_POSITION	    EQU     0d
ROW_SHIFT	    EQU     8d
COLUMN_SHIF0T     EQU     8d    
PACMAN_X          EQU     1d
PACMAN_Y          EQU     1d
DOWN              EQU     0d
UP                EQU     1d
LEFT              EQU     2d
RIGHT             EQU     3d
STOP              EQU     4d
TIMER_UNITS       EQU     FFF6h
ACTIVATE_TIMER    EQU     FFF7h
ON                EQU     1d
OFF               EQU     0d
POSICAO_INICIAL   EQU     8052h
POSICAO_FANTASMA1 EQU     80f7h
; padrao de bits para geracao de numero aleatorio
RND_MASK		EQU	8016h	; 1000 0000 0001 0110b
LSB_MASK		EQU	0001h	; Mascara para testar o bit menos significativo do Random_Var
PRIME_NUMBER_1	EQU 11d
PRIME_NUMBER_2	EQU 13d


;------------------------------------------------------------------------------
; ZONA II: definicao de variaveis
;          Pseudo-instrucoes : WORD - palavra (16 bits)
;                              STR  - sequencia de caracteres (cada ocupa 1 palavra: 16 bits).
;          Cada caracter ocupa 1 palavra
;------------------------------------------------------------------------------

                ORIG    8000h
L1  		  STR     '################################################################################', FIM_TEXTO
L2              STR     '#C      .#           #                  #    ## . .    #       #.       #      #', FIM_TEXTO
L3              STR     '#.    .          .....  .....  ..... #     ## .    .  #       #.       #       #', FIM_TEXTO
L4              STR     '#.     .#        .      .      .      #      ###    . #       #.       #....   #', FIM_TEXTO
L5              STR     '#     #..   ...  .....  ...... .....                  #  ...  ####     #####   #', FIM_TEXTO
L6              STR     '#.     .#     #  .   .  .    . .   .                .    .       #             #', FIM_TEXTO
L7              STR     '#.     .#     #  .....  ...... .....                             #             #', FIM_TEXTO
L8              STR     '#.      #######       ###############  ################          #####         #', FIM_TEXTO
L9              STR     '#.                    #                               #  ####         .....    #', FIM_TEXTO
L10             STR     '#.                    #                               #     #                  #', FIM_TEXTO
L11             STR     '#.    ####            #                               #     #        #         #', FIM_TEXTO
L12             STR     '#.       ##### ...                                             .       #########', FIM_TEXTO
L13             STR     '#.                    #                               ##        .              #', FIM_TEXTO
L14             STR     '#.     ###       ######                               ##       #####           #', FIM_TEXTO
L15             STR     '#.                    #                               #      .  #..#    #      #', FIM_TEXTO
L16             STR     '#.     #######       ################ ################         ..#.. .. #     .#', FIM_TEXTO
L17             STR     '#.        #                #      V                            ..########     .#', FIM_TEXTO 
L18             STR     '#.        # .....  ....  ...  ....  ....   #                                  .#', FIM_TEXTO
L19             STR     '#.        ##.   .  .     .  . .  .  .  .                        ##      #      #', FIM_TEXTO
L20             STR     '#         ##.   .  ....  .  . ...   .  .   .## ##.         #   ##  ..      #####', FIM_TEXTO
L21             STR     '#         ##.....  .     .  . .  .  .  .   .#   #.     ..    ..  #     ...    .#', FIM_TEXTO
L22             STR     '#         ##.      ...., ...  .   . ....   .## ##.  #   ..   .      #.    .    #', FIM_TEXTO
L23             STR     '################################################################################', FIM_TEXTO
L24             STR     'Pontos: 000                                                             Vidas: 3', FIM_MAPA

Derrota         STR     'VOCE PERDEU!',FIM_MAPA

Vitoria         STR     'VITORIA!',FIM_MAPA


RowIndex		WORD	0d
ColumnIndex		WORD	0d
TextIndex		WORD	0d
YFim                 WORD   11d

;-------------------------------------------------------------------------------
;Configurações Pacman
;-------------------------------------------------------------------------------
EnderecoString    WORD  0d
PacmanDirection   WORD  RIGHT
PacmanNovoX       WORD  PACMAN_X
PacmanNovoY       WORD  PACMAN_Y
PacmanEndereco    WORD  POSICAO_INICIAL

;-------------------------------------------------------------------------------
;Configurações pontos
;-------------------------------------------------------------------------------
Unidade           WORD  48d
Dezena            WORD  48d
Centena           WORD  48d
linha_Pontos      WORD  23d
coluna_Pontos     WORD  8d
flagVenceu        WORD  0d

;-------------------------------------------------------------------------------
;Configurações Vida
;-------------------------------------------------------------------------------

Vidas             WORD  51d
linha_vida        WORD  23d
coluna_vida       WORD  79d
Random_var        WORD  A5A5h

;-------------------------------------------------------------------------------
;Configurações dos Fantasmas
;-------------------------------------------------------------------------------
DirecaoGhost1   WORD    DOWN
XGhost1         WORD    4d
YGhost1         WORD    3d
PosGhost1       WORD    POSICAO_FANTASMA1

;------------------------------------------------------------------------------
; ZONA III: definicao de tabela de interrupções
;------------------------------------------------------------------------------
                ORIG    FE00h
INT0            WORD    MoverDireita
INT1            WORD    MoverEsquerda
INT2            WORD    MoverCima
INT3            WORD    MoverBaixo
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

                  CALL PrintPontos 
                  CALL VerificaColisaoFantasmaPacman   
                  
                  
                  CALL VerificaMovimentoGhost1
                  MOV R2, M[DirecaoGhost1]

                  ;-----------------------
                  ;Fantasma 1
                  ;-----------------------   

                  CMP R2, DOWN
                  CALL.Z MovimentaBaixoGhost1
                  CMP R2, UP 
                  CALL.Z MovimentaCimaGhost1
                  CMP R2, RIGHT
                  CALL.Z MovimentaDireitaGhost1
                  CMP R2, LEFT
                  CALL.Z MovimentaEsquerdaGhost1


                  CALL VerificaColisaoFantasmaPacman    
                  CALL VerificaMovimento
                  MOV R1, M[PacmanDirection]
                  
                  ;-----------------------
                  ;Pacman
                  ;-----------------------

                  CMP R1, RIGHT
                  CALL.Z MovimentaDireita
                  CMP R1, LEFT
                  CALL.Z MovimentaEsquerda
                  CMP R1, UP
                  CALL.Z MovimentaCima
                  CMP R1, DOWN
                  CALL.Z MovimentaBaixo

                  CALL WinLoseCheck
                  
                  
                  CALL Configura_Timer

                  POP R2
                  POP R1
                  RTI

;------------------------------------------------------------------------------
;Função que verifica se houve colisão com o fantasma 1
;------------------------------------------------------------------------------
VerificaColisaoFantasmaPacman:PUSH    R1
                              PUSH    R2
                              PUSH    R3

                              MOV     R1, M[PacmanNovoX]
                              MOV     R2, M[XGhost1]
                              CMP     R1, R2  ;Compara a posição X do fantasma e do PacMan
                              JMP.NZ  NaoMorreu
                              MOV     R1, M[PacmanNovoY]
                              MOV     R2, M[YGhost1]
                              CMP     R1, R2  ;Compara a posição Y do fantasma e do PacMan
                              JMP.NZ  NaoMorreu

                              ;Caso X e Y sejam iguais as posições dos fantasmas e do PacMan serão resetadas
                              
                              CALL ResetMapa
                              DEC     M[Vidas]
                              CALL    DiminuiLife

                              NaoMorreu:POP     R3
                                        POP     R2
                                        POP     R1
                                        RET

;------------------------------------------------------------------------------
;Funções de fim de jogo
;------------------------------------------------------------------------------

WinLoseCheck: PUSH R1 
              
              MOV R1, M[Vidas] ;Verifica se as vidas sao iguais a zero
              CMP R1, 48d
              
              JMP.Z Lose ; Se sim, perdeu
              
              MOV R1, M[Centena]   ;Verifica se a pontuacao =  212, se sim, ganhou
              CMP R1, 50d
              JMP.NZ NaoGanhou
              
              MOV R1, M[Dezena]
              CMP R1, 49d
              JMP.NZ NaoGanhou
              
              MOV R1, M[Unidade]
              CMP R1, 50d
              JMP.NZ NaoGanhou
              
              Jmp WIN
              
              POP R1
              RET 

Lose:POP R1
     CALL PrintVitDer  
     JMP Halt

NaoGanhou:POP R1
          RET  

WIN:    POP R1

        PUSH R1
        MOV R1, 1d
        MOV M[flagVenceu],R1
        CALL PrintVitDer
        POP R1
        
        JMP Halt

;------------------------------------------------------------------------------
;Função para imprimir a vitoria ou a derrota
;------------------------------------------------------------------------------
PrintVitDer:    PUSH    R1
                PUSH    R2
                PUSH    R3

                MOV     R1, 34d
                MOV     M[ColumnIndex],R1
                MOV     R1, M[ flagVenceu ]
                CMP     R1, 1d
                JMP.NZ  Perdeu 
                MOV     R1, Vitoria
                JMP     Continue

                Perdeu:         MOV     R1, Derrota

                Continue:       MOV     R3, M[ ColumnIndex ]
                                MOV     M[ TextIndex ], R1
                WhileFIM:       MOV     R2, M[ YFim ]
                                MOV     R1, M[ TextIndex ]
                                SHL     R2, 8d
                                MOV     R1, M[ R1 ]
                                OR      R2, R3
                                MOV     M[ CURSOR ], R2

                                INC     M[ TextIndex ]
                                INC     R3

                                CMP     R1, FIM_MAPA
                                JMP.Z  FimVitoria
                                MOV     M[ IO_WRITE ], R1
                                JMP     WhileFIM

FimVitoria:     POP     R3
                POP     R2
                POP     R1
                RET
               
;------------------------------------------------------------------------------
;Funções para quando o pacman colidir com o fantasma
;------------------------------------------------------------------------------

DiminuiLife: PUSH R1
             PUSH R2
             PUSH R3
             
             MOV   R1, M[ Vidas ]
             MOV   R2, M[linha_vida]
             MOV   R3, M[coluna_vida]
             SHL   R2, 8d
             OR    R2, R3
             MOV   M[ CURSOR ], R2
             MOV   M[ IO_WRITE ], R1 

             POP R3
             POP R2
             POP R1
             RET

ResetMapa: PUSH R1
           PUSH R2 

           MOV R1, M[PacmanEndereco] ;Limpa a posicao que o pacman estava
           MOV R2, ' '
           MOV M[R1], R2 ; Limpou a posicao na RAM 

           MOV R1, DOWN; direcao para baixo
           MOV M[PacmanDirection], R1

           MOV R2, PACMAN_X 
           MOV  M[PacmanNovoX], R2 ; Muda os valores de x e y do pacman

           MOV R2, PACMAN_Y
           MOV  M[PacmanNovoY], R2
           MOV R1, POSICAO_INICIAL 
           MOV  M[PacmanEndereco],R1
           MOV R2,'C'
           MOV M[R1],R2 

           POP R2
           POP R1
           RET

;------------------------------------------------------------------------------
;Funções de movimentação do pacman
;------------------------------------------------------------------------------

MoverDireita:     PUSH R1
                  
                  MOV R1, RIGHT
                  MOV M[PacmanDirection], R1

                  POP R1

                  RTI

MoverEsquerda:    PUSH R1
                  
                  MOV R1, LEFT
                  MOV M[PacmanDirection], R1

                  POP R1

                  RTI

MoverCima: PUSH R1
                  
           MOV R1, UP
           MOV M[PacmanDirection], R1

           POP R1

           RTI

MoverBaixo: PUSH R1
                  
           MOV R1, DOWN
           MOV M[PacmanDirection], R1

           POP R1

           RTI

MovimentaDireita: PUSH R1
                  PUSH R2

                  MOV R1, M[PacmanNovoY]
                  MOV R2, M[PacmanNovoX]

                  SHL R1, 8d
                  OR R1,R2
                  MOV M[CURSOR],R1
                  MOV R1, ' '
                  MOV M[IO_WRITE],R1

                  MOV R1, M[PacmanEndereco]

                  MOV R2, ' '   
                  MOV M[R1],R2


                  INC M[PacmanNovoX]
                  INC M[PacmanEndereco]

                  MOV R1,M[PacmanEndereco]
                  MOV R2, 'C'
                  MOV M[R1],R2 

                  MOV R1, M[PacmanNovoY]
                  MOV R2, M[PacmanNovoX]
                  SHL R1, 8d
                  OR R1,R2                  
                  MOV M[CURSOR], R1

                  MOV R1,'C'
                  MOV M[IO_WRITE],R1

                  POP R2
                  POP R1
                  RET

MovimentaEsquerda:PUSH R1
                  PUSH R2

                  MOV R1, M[PacmanNovoY]
                  MOV R2, M[PacmanNovoX]

                  SHL R1, 8d
                  OR R1,R2
                  MOV M[CURSOR],R1
                  MOV R1, ' '
                  MOV M[IO_WRITE],R1
                  
                  MOV R1, M[PacmanEndereco]
                  MOV R2, ' '   
                  MOV M[R1],R2

                  DEC M[PacmanNovoX]
                  DEC M[PacmanEndereco]

                  MOV R1,M[PacmanEndereco]
                  MOV R2, 'C'
                  MOV M[R1],R2 

                  MOV R1, M[PacmanNovoY]
                  MOV R2, M[PacmanNovoX]
                  SHL R1, 8d
                  OR R1,R2                  
                  MOV M[CURSOR], R1

                  MOV R1,'C'
                  MOV M[IO_WRITE],R1

                  POP R2
                  POP R1
                  RET

MovimentaBaixo:   PUSH R1
                  PUSH R2

                  MOV R1, M[PacmanNovoY]
                  MOV R2, M[PacmanNovoX]

                  SHL R1, 8d
                  OR R1,R2
                  MOV M[CURSOR],R1
                  MOV R1, ' '
                  MOV M[IO_WRITE],R1

                  INC M[PacmanNovoY]
                  MOV R1,M[PacmanEndereco]
                  MOV R2, ' '
                  MOV M[R1],R2
                  ADD R1,81d
                  MOV M[PacmanEndereco],R1
                  MOV R2, 'C'
                  MOV M[R1],R2

                  MOV R1, M[PacmanNovoY]
                  MOV R2, M[PacmanNovoX]
                  SHL R1, 8d
                  OR R1,R2                  
                  MOV M[CURSOR], R1

                  MOV R1,'C'
                  MOV M[IO_WRITE],R1

                  POP R2
                  POP R1
                  RET

MovimentaCima:    PUSH R1
                  PUSH R2

                  MOV R1, M[PacmanNovoY]
                  MOV R2, M[PacmanNovoX]

                  SHL R1, 8d
                  OR R1,R2
                  MOV M[CURSOR],R1
                  MOV R1, ' '
                  MOV M[IO_WRITE],R1

                  DEC M[PacmanNovoY]
                  MOV R1,M[PacmanEndereco]
                  MOV R2, ' '   
                  MOV M[R1],R2
                  SUB R1,81d
                  MOV M[PacmanEndereco],R1
                  MOV R2, 'C'
                  MOV M[R1],R2 

                  MOV R1, M[PacmanNovoY]
                  MOV R2, M[PacmanNovoX]
                  SHL R1, 8d
                  OR R1,R2                  
                  MOV M[CURSOR], R1

                  MOV R1,'C'
                  MOV M[IO_WRITE],R1

                  POP R2
                  POP R1
                  RET

VerificaMovimento: PUSH R1 
                   PUSH R2 

                   MOV R1, M[PacmanDirection]

                   CMP R1, RIGHT
                   CALL.Z VerificaDireita
                   CMP R1, LEFT 
                   CALL.Z VerificaEsquerda
                   CMP R1, UP
                   CALL.Z VerificaCima
                   CMP R1, DOWN
                   CALL.Z VerificaBaixo
                   
                   POP R2
                   POP R1
                   RET

VerificaDireita: PUSH R1
                 PUSH R2

                 MOV R1, M[PacmanEndereco]
                 INC R1
                 MOV R1, M[R1]
                 CMP R1,'#'
                 CALL.Z Parede
                 CMP R1,'.'
                 CALL.Z Ponto  
                 POP R2
                 POP R1
                 RET 

VerificaEsquerda: PUSH R1
                  PUSH R2
                  
                  MOV R1, M[PacmanEndereco]
                  DEC R1
                  MOV R1, M[R1]
                  CMP R1,'#'
                  CALL.Z Parede
                  CMP R1,'.'
                  CALL.Z Ponto
                  POP R2
                  POP R1
                  RET 

VerificaBaixo:    PUSH R1
                  PUSH R2
                  
                  MOV R1, M[PacmanEndereco]
                  ADD R1, 81d
                  MOV R1, M[R1]
                  CMP R1,'#'
                  CALL.Z Parede
                  CMP R1,'.'
                  CALL.Z Ponto
                  POP R2
                  POP R1
                  RET 

VerificaCima:    PUSH R1
                 PUSH R2
                  
                 MOV R1, M[PacmanEndereco]
                 SUB R1, 81d
                 MOV R1, M[R1]
                 CMP R1,'#'
                 CALL.Z Parede
                 CMP R1,'.'
                 CALL.Z Ponto
                 POP R2
                 POP R1
                 RET 

Parede:PUSH R5 
       MOV R5, STOP
       MOV M[PacmanDirection],R5
       POP R5
       RET

;------------------------------------------------------------------------------
;Funções de pontuação
;------------------------------------------------------------------------------

Ponto: PUSH R1
       PUSH R2
       PUSH R3
       PUSH R4
       PUSH R5

       MOV R2, M[PacmanEndereco]
       INC M[Unidade]

       MOV R5, M[PacmanDirection]
       CMP R5, RIGHT
       CALL.Z PontoDireita
       CMP R5, LEFT
       CALL.Z PontoEsquerda
       CMP R5, UP
       CALL.Z PontoCima
       CMP R5, DOWN
       CALL.Z PontoBaixo


       MOV R3, 58d
       MOV R4, M[Unidade]

       CMP R3,R4
       CALL.Z aumentaDezena

       CALL PrintPontos

       SHL R1, 3


       POP R5
       POP R4
       POP R3
       POP R2
       POP R1 
       RET  

PontoDireita: PUSH R1
              PUSH R2

              MOV R2, M[PacmanEndereco]
              MOV R1,' '
              INC R2
              MOV M[R2], R1

              POP R2
              POP R1
              RET

PontoEsquerda:PUSH R1
              PUSH R2

              MOV R2, M[PacmanEndereco]
              MOV R1,' '
              DEC R2
              MOV M[R2], R1

              POP R2
              POP R1
              RET

PontoCima:PUSH R1
          PUSH R2

          MOV R2, M[PacmanEndereco]
          MOV R1,' '
          
          SUB R2, 81d

          MOV M[R2], R1

          POP R2
          POP R1
          RET

PontoBaixo:PUSH R1
           PUSH R2

           MOV R2, M[PacmanEndereco]
           MOV R1,' '
          
           ADD R2, 81d

           MOV M[R2], R1

           POP R2
           POP R1
           RET

aumentaDezena:PUSH R1
              PUSH R2

              MOV R1, 48d
              MOV M[Unidade],R1
              INC M[Dezena]
              MOV R2, 58d
              MOV R1,M[Dezena]
              CMP R1,R2
              CALL.Z aumentaCentena

              POP R2
              POP R1
              RET

aumentaCentena:PUSH R1
               PUSH R2

               MOV R1, 48d
               MOV M[Dezena],R1
               INC M[Centena]

               POP R2
               POP R1
               RET


;------------------------------------------------------------------------------
;Funções de movimentação do fantasma 1
;------------------------------------------------------------------------------

VerificaMovimentoGhost1:PUSH R1 
                        PUSH R2 

                        MOV R1, M[DirecaoGhost1]

                        CMP R1, DOWN  
                        CALL.Z VerificaBaixoGhost1
                        CMP R1, UP
                        CALL.Z VerificaCimaGhost1
                        CMP R1,RIGHT
                        CALL.Z VerificaDireitaGhost1
                        CMP R1,LEFT
                        CALL.Z VerificaEsquerdaGhost1
                   
                        POP R2
                        POP R1
                        RET

MovimentaBaixoGhost1:PUSH R1
                     PUSH R2

                     MOV R1, M[YGhost1]
                     MOV R2, M[XGhost1]

                     SHL R1, 8d ;Escrevo na tela o que tem naquela posicao, espaço ou comida
                     OR R1,R2                  
                     MOV M[CURSOR], R1
                     MOV R2,M[PosGhost1]
                     MOV R1,M[R2]
                     MOV M[IO_WRITE],R1

                     INC M[YGhost1] ;Mudo a posicao do fantasma 
                     MOV R1,M[PosGhost1]
                     ADD R1,81d
                     MOV M[PosGhost1],R1

                     MOV R1, M[YGhost1] 
                     MOV R2, M[XGhost1]
                     SHL R1, 8d
                     OR R1,R2                  
                     MOV M[CURSOR], R1
                     MOV R1,'V'
                     MOV M[IO_WRITE],R1 ;Printo o fantasma na nova posicao 

                     POP R2
                     POP R1
                     RET

MovimentaCimaGhost1:PUSH R1
                     PUSH R2

                     MOV R1, M[YGhost1]
                     MOV R2, M[XGhost1]

                     SHL R1, 8d ;Escrevo na tela o que tem naquela posicao, espaço ou comida
                     OR R1,R2                  
                     MOV M[CURSOR], R1
                     MOV R2,M[PosGhost1]
                     MOV R1,M[R2]
                     MOV M[IO_WRITE],R1

                     DEC M[YGhost1] ;Mudo a posicao do fantasma
                     MOV R1,M[PosGhost1]
                     SUB R1,81d
                     MOV M[PosGhost1],R1

                     MOV R1, M[YGhost1] ;Escrevo na tela na nova posicao
                     MOV R2, M[XGhost1]
                     SHL R1, 8d
                     OR R1,R2                  
                     MOV M[CURSOR], R1
                     MOV R1,'V'
                     MOV M[IO_WRITE],R1

                     POP R2
                     POP R1
                     RET

MovimentaDireitaGhost1:PUSH R1
                       PUSH R2

                     MOV R1, M[YGhost1]
                     MOV R2, M[XGhost1]

                     SHL R1, 8d ;Escrevo na tela o que tem naquela posicao, espaço ou comida
                     OR R1,R2                  
                     MOV M[CURSOR], R1
                     MOV R2,M[PosGhost1]
                     MOV R1,M[R2]
                     MOV M[IO_WRITE],R1                     

                     INC M[XGhost1] ;Mudo a posicao do fantasma
                     MOV R1,M[PosGhost1]
                     ADD R1,1d
                     MOV M[PosGhost1],R1

                     MOV R1, M[YGhost1] ;Escrevo na tela na nova posicao
                     MOV R2, M[XGhost1]
                     SHL R1, 8d
                     OR R1,R2                  
                     MOV M[CURSOR], R1
                     MOV R1,'V'
                     MOV M[IO_WRITE],R1

                     POP R2
                     POP R1
                     RET

MovimentaEsquerdaGhost1:PUSH R1
                        PUSH R2

                     MOV R1, M[YGhost1]
                     MOV R2, M[XGhost1]

                     SHL R1, 8d ;Escrevo na tela o que tem naquela posicao, espaço ou comida
                     OR R1,R2                  
                     MOV M[CURSOR], R1
                     MOV R2,M[PosGhost1]
                     MOV R1,M[R2]
                     MOV M[IO_WRITE],R1                                  

                     DEC M[XGhost1] ;Mudo a posicao do fantasma
                     MOV R1,M[PosGhost1]
                     DEC R1
                     MOV M[PosGhost1],R1

                     MOV R1, M[YGhost1] ;Escrevo na tela na nova posicao
                     MOV R2, M[XGhost1]
                     SHL R1, 8d
                     OR R1,R2                  
                     MOV M[CURSOR], R1
                     MOV R1,'V'
                     MOV M[IO_WRITE],R1

                     POP R2
                     POP R1
                     RET

VerificaBaixoGhost1:PUSH R1
                   PUSH R2
                  
                   MOV R1, M[PosGhost1]
                   ADD R1, 81d
                   MOV R1, M[R1]
                   CMP R1,'#'
                   CALL.Z ParedeGhost
                   POP R2
                   POP R1
                   RET 

VerificaCimaGhost1:PUSH R1
                   PUSH R2
                   MOV R1, M[PosGhost1]
                   SUB R1, 81d
                   MOV R1, M[R1]
                   CMP R1,'#'
                   CALL.Z ParedeGhost
                   POP R2
                   POP R1
                   RET 

VerificaDireitaGhost1:PUSH R1
                   PUSH R2
                  
                   MOV R1, M[PosGhost1]
                   INC R1
                   MOV R1, M[R1]
                   CMP R1,'#'
                   CALL.Z ParedeGhost
                   POP R2
                   POP R1
                   RET 

VerificaEsquerdaGhost1:PUSH R1
                       PUSH R2
                  
                       MOV R1, M[PosGhost1]
                       DEC R1
                       MOV R1, M[R1]
                       CMP R1,'#'
                       CALL.Z ParedeGhost
                       POP R2
                       POP R1
                       RET 

ParedeGhost:PUSH R5 
            MOV R5, STOP
            MOV M[DirecaoGhost1],R5 
            CALL Random
            CALL VerificaMovimentoGhost1
            POP R5
            RET
;------------------------------------------------------------------------------
;Random (Gera número aleatório)
;------------------------------------------------------------------------------
Random:         PUSH    R1
                PUSH    R2
                MOV     R1, LSB_MASK
                AND     R1, M[Random_var]
                BR.Z    Rnd_rotate
                MOV     R1, RND_MASK
                XOR     M[Random_var], R1

Rnd_rotate:     ROR     M[Random_var], 1
                     
                MOV     R2, 4d
                MOV     R1, M[ Random_var]
                DIV     R1, R2                  ;Fazemos uma divisão por 4 pois existem apenas 4 direções
                MOV     M[ DirecaoGhost1 ], R2 ;O resto da divisão é um valor entre 0 e 3 

                POP     R2
                POP     R1
                RET
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

PrintPontos:PUSH R1
            PUSH R2
            PUSH R3 
            PUSH R4

            MOV   R1, M[ Centena ]
            MOV   R2, M[ linha_Pontos ]
            MOV   R3, M[ coluna_Pontos ]
            SHL   R2, 8d
            OR    R2, R3
            MOV   M[ CURSOR ], R2
            MOV   M[ IO_WRITE ], R1 

            MOV R4, M[coluna_Pontos]
            INC R4

            MOV   R1, M[ Dezena ]
            MOV   R2, M[linha_Pontos]
            MOV   R3, R4
            SHL   R2, 8d
            OR    R2, R3
            MOV   M[ CURSOR ], R2
            MOV   M[ IO_WRITE ], R1 

            INC R4

            MOV   R1, M[ Unidade ]
            MOV   R2, M[linha_Pontos]
            MOV   R3, R4
            SHL   R2, 8d
            OR    R2, R3
            MOV   M[ CURSOR ], R2
            MOV   M[ IO_WRITE ], R1 

            POP R4
            POP R3 
            POP R2
            POP R1
            RET

;------------------------------------------------------------------------------
; Função para configurar o timer
;------------------------------------------------------------------------------

Configura_Timer:  PUSH R1
                  PUSH R2

                  MOV R1,5d
                  MOV M[TIMER_UNITS], R1
                  MOV R1, ON
                  MOV M[ACTIVATE_TIMER], R1

                  POP R2
                  POP R1
                  RET

;------------------------------------------------------------------------------
; Função Main
;------------------------------------------------------------------------------
Main:			ENI

				MOV		R1, INITIAL_SP
				MOV		SP, R1		 		; We need to initialize the stack
				MOV		R1, CURSOR_INIT		; We need to initialize the cursor 
				MOV		M[ CURSOR ], R1		; with value CURSOR_INIT

				CALL PrintarMapa

                            ;Configura o fantasma 1
                            MOV R1,M[XGhost1]; Coluna
                            MOV R2,M[YGhost1] ; Linha 
                            MOV R3,81d ; Multiplica linha pelo tamanho da linha
                            MUL R2,R3 ; Fica guardado em R3
                            ADD R3,R1 ; R3 é o deslocamento
                            MOV R1,L1
                            ADD R1,R3
                            MOV M[PosGhost1],R1

                            ;Printa o fantasma 1 na tela 
                            MOV R1, M[YGhost1] 
                            MOV R2, M[XGhost1]
                            SHL R1, 8d
                            OR R1,R2                  
                            MOV M[CURSOR], R1
                            MOV R1,'V'
                            MOV M[IO_WRITE],R1 ;Printo o fantasma na nova posicao 



                            MOV R1,5d
                            MOV M[TIMER_UNITS],R1
                            MOV R1,ON
                            MOV M[ACTIVATE_TIMER],R1






Cycle: 			BR		Cycle	
Halt:           BR		Halt