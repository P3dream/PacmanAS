/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Main memory.
|
| History: 17/mar/01 - JCM - created.
|          11/nov/01 - JCM - new IO ports
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                       Arlindo Oliveira, aml@inesc.pt
|                 	Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/


#include "defs_sim.h"


static word main_memory[MAIN_MEM_SIZE];
static unsigned char io_read_buffer = 0;/*Holds last key pressed in IO window */

/*-----------------------------------------------------------------------*/
int read_main_memory(mem_addr addr, word *mem_data)
{
    if(addr >= IO_ADDR)
	switch(addr){
	  case IO_TEXT_READ:
	    *mem_data = (word) io_read_buffer;
            io_read_buffer = 0;
	    break;
	  case IO_TEXT_STATUS:
	    *mem_data = (word) (io_read_buffer != 0);
	    break;
          case IO_SWITCHES:
	    *mem_data = io_read_switches();
	    break;
	  case TIMER_UNITS:
	    *mem_data = read_units();
	    break;
	  case TIMER_START:
	    *mem_data = read_start();
	     break;
	  case MASK_ADDR:
	    *mem_data = read_mask();  
	    break;
	  default:
            *mem_data = VALUE_READ_FROM_WRITE_PORTS;
	}
    else
        *mem_data = main_memory[addr];

    return 0;
}



/*-----------------------------------------------------------------------
Writes to read ports are ignored                                       
-------------------------------------------------------------------------*/
void write_main_memory(mem_addr addr, word mem_data)
{
    if(addr >= IO_ADDR)
	switch(addr){

          case IO_TEXT_WRITE:
	    writeIOText(0, mem_data);
	    return;
	  case IO_TEXT_CONTROL:
	    writeIOText(1, mem_data);
	    return;
          case IO_LCD_WRITE:
	    writeLCDText(0, mem_data);
	    return;
	  case IO_LCD_CONTROL:
	    writeLCDText(1, mem_data);
	    return;
	  case IO_LEDS:
	    writeLEDs(mem_data);
	    return;
	  case IO_SEVEN_SEG3:
	    write7seg(1, mem_data);
	    return;
	  case IO_SEVEN_SEG2:
	    write7seg(2, mem_data);
	    return;
	  case IO_SEVEN_SEG1:
	    write7seg(3, mem_data);
	    return;
	  case IO_SEVEN_SEG0:
	    write7seg(4, mem_data);
	    return;
	  case MASK_ADDR:
	    writeMASK(mem_data);
	    return;   
	  case TIMER_START:
	    refresh(mem_data);
	    writeTIMER(0, mem_data);
	    return;
	  case TIMER_UNITS:
	    writeTIMER(1, mem_data);
 	}
    else
        main_memory[addr] = mem_data;
}


/*-----------------------------------------------------------------------*/
void newIOinput(char c)
{
    io_read_buffer = c;
}


/*-------------------------------------------------------------------------
Function called by the interface. Returns one string with the requested
memory positions, in hexadecimal, starting at the address given, with
'n_words' per line.
-------------------------------------------------------------------------*/
int content_main_memory(int addr, int n_words, char *mem_data)
{
    int i;
    char line[LINE_LENGTH],
         ascii[LINE_LENGTH];

    if((addr < 0) || ((addr + n_words) > MAIN_MEM_SIZE) || (n_words < 1))
        return 1;

    sprintf(mem_data, "%4.4hx", main_memory[addr]);
    if((main_memory[addr] >= 32) && (main_memory[addr] < 127))
        sprintf(ascii, "%c", main_memory[addr]);
    else
        sprintf(ascii, ".");
    for(i = 1; i < n_words; i++){
        sprintf(line, "  %4.4hx", main_memory[addr+i]);
        strcat(mem_data, line);
        if((main_memory[addr+i] >= 32) && (main_memory[addr+i] < 127))
            sprintf(ascii, "%s%c", ascii, main_memory[addr+i]);
        else
            strcat(ascii, ".");
    }
    sprintf(mem_data, "%s      %s", mem_data, ascii);

    return 0;
}
