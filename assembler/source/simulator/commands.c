/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Routines called from the interface.
|
| History: 17/mar/01 - JCM - created.
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                 	Arlindo Oliveira, aml@inesc.pt
|                       Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/


#include <stdio.h>
#include "defs_inter.h"

#ifdef _WIN32
#define READ_BIN "rb"
#define WRITE_BIN "wb"
#else
#define READ_BIN "r"
#define WRITE_BIN "w"
#endif

#define MAX_LINE_LEN 1000
#define MAX_LCD_COLUMN 16
#define MAX_LCD_LINE 3

#define TIMER_INTERVAL_MILI	100

static ivad_index ivad[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8 , 9, 10, 11, 12, 13, 14, 15};
static int active_iv[15];   /* Set by interface */

static char line[MAX_LINE_LEN];

static char positionFlag = 0; /* Indicates that a position in the IO window
                                 has been specified and next character will
                                 be printed there */

static char LCD_on = 1;
static int LCD_line = 1;
static int LCD_column = 0;
static int clear;

static int MASK = 0;
static word TIME = 0;
static word START = 0;

#ifdef _WIN32   /* Windows version */
UINT_PTR windowsTimerID = 0;
DWORD previousTime = 0;
DWORD firstTime = 0;
#else   /* Unix version */
static struct itimerval count;
#endif

static char mat_lcd[2][16]={{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
/*-----------------------------------------------------------------------*/
int memContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int status;

    if(argc != 2)
        return TCL_ERROR;

    status = content_main_memory(atoi(argv[1]), MEM_WORDS_PER_LINE,
                                 interp->result);

/*    sprintf(interp->result,"%4x  %4x  %4x  %4x  %4x  %4x  %4x  %4x ",random() % 65536,random() % 65536,random() % 65536,random() % 65536,random() % 65536,random() % 65536,random() % 65536,random() % 65536);*/

    return (status ? TCL_ERROR : TCL_OK);
}


/*-----------------------------------------------------------------------*/
int regContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int status;

    if(argc != 2)
        return TCL_ERROR;

    status = reg_content(atoi(argv[1]), interp->result);

    return (status ? TCL_ERROR : TCL_OK);
}


/*-----------------------------------------------------------------------*/
int pcContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    pc_content(interp->result);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int spContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    sp_content(interp->result);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int reContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int flag;

    if(argc != 2)
        return TCL_ERROR;

    flag = atoi(argv[1]);
    if((flag < 0) || (flag > 5))
        return TCL_ERROR;

    if(test_RE(flag))
        sprintf(interp->result, "1");
    else
        sprintf(interp->result, "0");

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int ureContent(ClientData clientData, Tcl_Interp *interp,int argc, const char *argv[])
{
    int flag;

    if(argc != 2)
        return TCL_ERROR;

    flag = atoi(argv[1]);
    if((flag < 0) || (flag > 4))
        return TCL_ERROR;

    if(test_uRE(flag))
        sprintf(interp->result, "1");
    else
        sprintf(interp->result, "0");

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int carContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    ctrlROM_addr car;

    car = car_content();

    sprintf(interp->result, "%3.3hx", car);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int irContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    ir_content(interp->result);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int sbrContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    sbr_content(interp->result);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int stepProc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int i = 0,
        status;

    do{
        status = execute1clock_cycle();
        if(status)
            return TCL_ERROR;
        i++;

    } while(car_content() != 0);

    sprintf(interp->result, "%d", i);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int clockProc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int status;

    status = execute1clock_cycle();

    /* Indicate if an instruction has finished */
    sprintf(interp->result, "%d", car_content() == 0);

    return (status ? TCL_ERROR : TCL_OK);
}


/*-----------------------------------------------------------------------*/
int unassembleAddr(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    mem_addr addr;
    char *inst;

    if(argc != 2)
        return TCL_ERROR;

    addr = (mem_addr)atoi(argv[1]);

    inst = get_instruction(addr);
    if(inst == NIL(char))
        sprintf(interp->result, "");
    else
        sprintf(interp->result, "%s", inst);

    return TCL_OK;
}




/*-----------------------------------------------------------------------
To avoid compatibilty problems between machines, we read one char at a time
-------------------------------------------------------------------------*/
int loadProgram(ClientData clientData, Tcl_Interp *interp,int argc, const char *argv[])
{
    FILE *prog_file;
    unsigned long mark;
    mem_addr addr,
	     size,
             code_addr,
             pc_addr,
             data;
    unsigned char c;
    int i;

    if(argc != 2)
        return TCL_ERROR;

    prog_file = fopen(argv[1], READ_BIN);
    if(prog_file == NIL(FILE)){
        sprintf(interp->result, "Cannot open program file: %s", argv[1]);
        return TCL_ERROR;
    }

    /* Check if this file was generated by the current version of p3as */
    mark = 0;
    for(i = 0; i < 4; i++){
        if(fread(&c, sizeof(char), 1, prog_file) != 1)
            goto error;
        mark |= ((unsigned long)c << 8*i);
    }
    if(mark != P3AS_MARK){
        sprintf(interp->result, "Object file %s was not generated by current version of 'p3as'!",
                argv[1]);
        return TCL_ERROR;
    }

    /* And now the code itself */
    while(1){
	if(fread(&c, sizeof(char), 1, prog_file) != 1)
	    goto error;
	size = c;
        if(fread(&c, sizeof(char), 1, prog_file) != 1)
            goto error;
        size |= ((unsigned short)c << 8);  /* Size of block of code */
	if(size == 0)
	    break;       /* Reached end of code */

	if(fread(&c, sizeof(char), 1, prog_file) != 1)
	    goto error;
	addr = c;
        if(fread(&c, sizeof(char), 1, prog_file) != 1)
            goto error;
        addr |= ((unsigned short)c << 8);  /* Start address of block */

	for(i = 0; i < size; i++){
	    if(fread(&c, sizeof(char), 1, prog_file) != 1)
		goto error;
	    data = c;
	    if(fread(&c, sizeof(char), 1, prog_file) != 1)
		goto error;
	    data |= ((unsigned short)c << 8);
	    write_main_memory(addr++, data);
	}
    }

    fclose(prog_file);

    reset_uP();
    return TCL_OK;

  error:
    sprintf(interp->result, "Error reading program file: %s", argv[1]);
    return TCL_ERROR;
}


/*-----------------------------------------------------------------------*/
int dumpMemory(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    FILE *mem_file;
    unsigned int addr;
    word data;

    if(argc != 2)
        return TCL_ERROR;

    mem_file = fopen(argv[1], "w");
    if(mem_file == NIL(FILE)){
        sprintf(interp->result, "Cannot open memory file: %s", argv[1]);
        return TCL_ERROR;
    }

    for(addr = 0; addr < MAIN_MEM_SIZE; addr++){
        read_main_memory((mem_addr) addr, &data);
        fprintf(mem_file, "%4.4hx %4.4hx\n", (mem_addr) addr, data);
    }
    fclose(mem_file);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int loadMemory(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    FILE *mem_file;
    unsigned int addr;
    int data,
        code_begin, code_end;

    if(argc != 2)
        return TCL_ERROR;

    mem_file = fopen(argv[1], "r");
    if(mem_file == NIL(FILE)){
        sprintf(interp->result, "Cannot open memory file: %s", argv[1]);
        return TCL_ERROR;
    }

    while(fgets(line, MAX_LINE_LEN, mem_file)){
        sscanf(line, "%x %x", &addr, &data);
        if(addr >= MAIN_MEM_SIZE){
            sprintf(interp->result, "Address too large: %x", addr);
            return TCL_ERROR;
        }
        if(data & ~0xffff){
            sprintf(interp->result,
                    "Value does not fit in a memory position: %x", data);
            return TCL_ERROR;
        }
        write_main_memory((mem_addr) addr, (word) data);
    }
    fclose(mem_file);

    reset_uP();

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int loadControlRom(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    FILE *rom_file;
    unsigned int addr;
    unsigned long data;

    if(argc != 2)
        return TCL_ERROR;

    rom_file = fopen(argv[1], "r");
    if(rom_file == NIL(FILE)){
        sprintf(interp->result, "Cannot open ROM file: %s", argv[1]);
        return TCL_ERROR;
    }

    while(fgets(line, MAX_LINE_LEN, rom_file)){
        sscanf(line, "%x %lx", &addr, &data);
        if(addr >= N_POS_ROM_CONTROL){
            sprintf(interp->result, "Address too large: %x", addr);
            return TCL_ERROR;
        }
        if(data & ~0xffffffff){
            sprintf(interp->result, "Value does not fit in ROM: %lx", data);
            return TCL_ERROR;
        }
        write_control_ROM((ctrlROM_addr) addr, (dword) data);
    }
    fclose(rom_file);

    reset_uP();

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int loadMapRom(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    FILE *rom_file;
    unsigned int addr,
                 max_addr;
    unsigned int data;
    void (*write_mapping_ROM)(word, ctrlROM_addr);

    if(argc != 3)
        return TCL_ERROR;

    rom_file = fopen(argv[1], "r");
    if(rom_file == NIL(FILE)){
        sprintf(interp->result, "Cannot open ROM file: %s", argv[1]);
        return TCL_ERROR;
    }

    switch(argv[2][0]){
      case 'A':
        write_mapping_ROM = write_ROMA;
        max_addr = N_POS_ROM_MAP_A;
        break;
      case 'B':
        write_mapping_ROM = write_ROMB;
        max_addr = N_POS_ROM_MAP_B;
        break;
      default:
        sprintf(interp->result, "Invalid mapping ROM: '%c', must be 'A' or 'B'",
                argv[2][0]);
        return TCL_ERROR;
    }

    while(fgets(line, MAX_LINE_LEN, rom_file)){
        sscanf(line, "%x %x", &addr, &data);
        if(addr >= max_addr){
            sprintf(interp->result, "Address too large: %x", addr);
            return TCL_ERROR;
        }
        if(data & ~0x1ff){
            sprintf(interp->result, "Value does not fit in ROM: %x", data);
            return TCL_ERROR;
        }
        write_mapping_ROM((word) addr, (ctrlROM_addr) data);
    }
    fclose(rom_file);

    reset_uP();

    return TCL_OK;
}

/*------------------------------------------------------------------------*/
void time_ivec(){

    if(MASK & 0x8000)
        interrupt(ivad[15]);
}

void nothing()
{
}

#ifdef _WIN32
/* decrement - Windows version */

VOID CALLBACK decrement(HWND hwnd,  UINT message, UINT iTimerID, DWORD dwTime) {
	/* WARNING: TIME � word e este � unsigned!! */
	int quantas;

	/* Calcular quantos intervalos tirar = time_elapsed / time_interval */
	quantas = (dwTime - previousTime) / TIMER_INTERVAL_MILI;

	TIME = (TIME <= quantas)?0:(TIME-quantas);

	/* START DEBUG ZONE */
/*	if(quantas>1) {
		printf("|   Timer%d (%d) ",iTimerID, quantas);
		printf("(%d->%d dif:%d)  |",previousTime, dwTime, dwTime-previousTime);
	} else {
		printf(".%d",((dwTime- previousTime) % TIMER_INTERVAL_MILI));
	}
*/	/* END DEBUG ZONE */

	/*previousTime = dwTime - ((dwTime - previousTime) - (quantas * TIMER_INTERVAL_MILI));*/
	previousTime = dwTime - ((dwTime - previousTime) % TIMER_INTERVAL_MILI);

	if(!TIME){
		time_ivec();

		/* START DEBUG ZONE */
/*		printf("--ACABOU %d--   ",iTimerID);
		printf("(%d->%d dif:%d=%d)       ",firstTime, dwTime, dwTime-firstTime,(dwTime-firstTime)/TIMER_INTERVAL_MILI);
*/		/* END DEBUG ZONE */

		/** Desactivar o timer */
		KillTimer(NULL, windowsTimerID);
		windowsTimerID = 0;
	}
}

#else
/* decrement - Unix version */

void decrement()
{
   TIME--;

   if(!TIME){
       time_ivec();

       count.it_interval.tv_usec = 0;
       count.it_value.tv_usec = 0;
       signal(SIGALRM, nothing);
       setitimer(ITIMER_REAL, &count,0);
   }
}

#endif



/*-----------------------------------------------------------------------*/
int resetPC(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{

    extern Tcl_Interp *g_interp;
    char command[16];
    int k, j;

    LCD_column = MASK  = 0;
    LCD_line = LCD_on = 1;
    for(k = 0; k < 2; k++)
        for(j = 0; j < 16; j++)
            mat_lcd[k][j] = 0;
    sprintf(command, "initPos_lcd");

    Tcl_Eval(g_interp, command);

    reset_uP();

		/* START DEBUG ZONE */
		/*** APAGAR    APAGAR    APAGAR    APAGAR    APAGAR    APAGAR    APAGAR   */
		/*TIME = 200;
		previousTime = GetTickCount();
		firstTime = previousTime;
		windowsTimerID = SetTimer(NULL, 0, TIMER_INTERVAL_MILI, decrement);*/
		/*** APAGAR    APAGAR    APAGAR    APAGAR    APAGAR    APAGAR    APAGAR   */
		/* END DEBUG ZONE */

    return 0;
}


/*-----------------------------------------------------------------------*/
int setRegisterValue(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    long value;
    int reg,
        status = 0;

    if(argc != 3)
        return TCL_ERROR;

    value = strtol(argv[2], NIL(char *), 16);
    if((value < -32768) || (value > 65535)){
        sprintf(interp->result, "Value does not fit in a register: %s",argv[2]);
        return TCL_ERROR;
    }

    if(!strcmp(argv[1], "PC"))
        set_register(PC_R15, value);
    else if(!strcmp(argv[1], "SP"))
        set_register(SP_R14, value);
    else if(!strcmp(argv[1], "RE"))
        set_RE(value);
    else if(argv[1][0] == 'R'){
        reg = atoi(&argv[1][1]);
        if((reg < 0) || (reg > 15)){
            sprintf(interp->result, "Invalid register index: %s", argv[1]);
            return TCL_ERROR;
        }
        if(reg == 0){
            sprintf(interp->result, "Cannot change value of register 0.");
            return TCL_ERROR;
        }
        status = set_register(reg, value);
    }
    else{
        sprintf(interp->result, "Invalid register: %s", argv[1]);
        return TCL_ERROR;
    }

    return (status ? TCL_ERROR : TCL_OK);
}


/*-----------------------------------------------------------------------*/
int setMemoryValue(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    long addr,
         value;
    int status = 0;

    if(argc != 3)
        return TCL_ERROR;

    addr = strtol(argv[1], NIL(char *), 16);
    if((addr < 0) || (addr > MAIN_MEM_SIZE)){
        sprintf(interp->result, "Invalid memory position: %s", argv[1]);
        return TCL_ERROR;
    }

    value = strtol(argv[2], NIL(char *), 16);
    if((value < -32768) || (value > 65535)){
        sprintf(interp->result, "Value does not fit in a memory position: %s",
                argv[2]);
        return TCL_ERROR;
    }

    write_main_memory((mem_addr) addr, (word) value);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int interruptProc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int ivec, mask_act;

    if(argc != 2)
        return TCL_ERROR;

    ivec = atoi(argv[1]);
    if((ivec < 0) || (ivec > 14)){
        sprintf(interp->result, "Invalid interrupt vector: %s", argv[1]);
        return TCL_ERROR;
    }

    mask_act = MASK & (1 << ivec);

    if(mask_act && active_iv[ivec])  /* Check if this interrupt vector is active */
        interrupt(ivad[ivec]);

    return TCL_OK;
}

/*-----------------------------------------------------------------------*/
int ioInput(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if(argc != 2)
        return TCL_ERROR;

    newIOinput(argv[1][0]);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
word io_read_switches()
{
    extern Tcl_Interp *g_interp;
    char command[] = "ReadSwitches";

    Tcl_Eval(g_interp, command);

    return atoi(g_interp->result);
}

word read_mask()
{
    return MASK;
}

word read_units()
{
    return TIME;
}

word read_start()
{
    return START;
}

/*-----------------------------------------------------------------------
Called from write_main_memory,
if flag control is 0, output character c to the xterm window
else it controls the cursor position on the window:
     if c is all 1's, put text window in positioning mode
     else 7 msb indicate line, 8 lsb indicate column
/*-----------------------------------------------------------------------*/
void writeIOText(char control, word c)
{
    extern Tcl_Interp *g_interp;
    char command[16];
    static int line, column;

    if(control){
        if(c == 0xffff){
            sprintf(command, "initPos");
	    line = column = 0;
	    positionFlag ^= 1;
	}
        else{
            /* 7 msb bit indicate line, 8 lsb indicate column */
            line = (c & 0x7f00) >> 8;
	    line++;
            column = c & 0xff;
	    positionFlag = 1;
            return;
        }
    }
    else
       	if(positionFlag)
	    if(c == '\\')
		// Hack: for some reason cannot read \\ in Tcl...
		sprintf(command, "putAt %d %d {%c}", column, line, 6);
	    else
		sprintf(command, "putAt %d %d {%c}", column, line, c);
	else
	    if(c == '\\')
		// Hack: for some reason cannot read \\ in Tcl...
		sprintf(command, "ioOutput {%c}", 6);
	    else{	
		sprintf(command, "ioOutput {%c}", c);
}
    Tcl_Eval(g_interp, command);
}



/*-----------------------------------------------------------------------
Called from write_main_memory,
if flag control is 0, output character c to the xterm window
else it controls the cursor position on the window:
      5th msb indicate line, 4 lsb indicate column
/*-----------------------------------------------------------------------*/
void writeLCDText(char control, word c)
{
    extern Tcl_Interp *g_interp;
    char command[32];
    int k, j, on_temp;

    if(control){
	LCD_line = (c & 0x10) >> 4;
        LCD_line++;
        LCD_column = c & 0xf;
	on_temp = LCD_on;
	LCD_on = (c & 0x8000) >> 15;
	clear = (c & 0x20) >> 5;
	if(clear){
	    for(k = 0; k < 2; k++)       // limpa texto no buffer
		for(j = 0; j < 16; j++)
		    mat_lcd[k][j] = 0;
	    if(LCD_on){                  // limpa visor, se activo
		sprintf(command, "initPos_lcd");
		Tcl_Eval(g_interp, command);
	    }
	    LCD_column = 0;
	    LCD_line = 1;
	}
	else
	    if(LCD_on){
		if(!on_temp)       // acende visor: copia texto do buffer
		    for(k = 0; k < 2; k++)
			for(j = 0; j < 16; j++){
			    sprintf(command, "putAt_lcd %d %d {%c}", j, k+1,
				    mat_lcd[k][j]);
			    Tcl_Eval(g_interp, command);
			}
	    }
	    else
		if(on_temp){      // apaga visor: limpa
		    sprintf(command, "initPos_lcd");
		    Tcl_Eval(g_interp, command);
		}
    }
    else{
	mat_lcd[LCD_line-1][LCD_column] = (char) c;
	if(LCD_on){
	    sprintf(command, "putAt_lcd %d %d {%c}", LCD_column, LCD_line, c);
	    Tcl_Eval(g_interp, command);
//printf("Escrever na coluna %d, linha %d\n", LCD_column, LCD_line);
        }
    }
}


/*-----------------------------------------------------------------------*/
void writeLEDs(word w)
{
    extern Tcl_Interp *g_interp;
    char command[64];
    int i, v[16], mask = 1;

    for(i = 0; i < 16; i++){
	v[i] = 0x0001 & ((w & mask) >> i);
    	mask <<= 1;
    }
    sprintf(command, "DisplayLeds %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			v[15], v[14], v[13], v[12], v[11], v[10], v[9], v[8], v[7], v[6], v[5], v[4], v[3], v[2], v[1], v[0]);

    Tcl_Eval(g_interp, command);
}


/*-----------------------------------------------------------------------*/
void write7seg(int index, word w)
{
    extern Tcl_Interp *g_interp;
    char command[64];
    int i,
	v[7],
	mask = 1,
	mask2 = 15,
	code[] = {63 , 6, 91, 79, 102, 109, 124, 7, 127, 111, 119, 124, 57, 94, 121, 113};

     for(i = 0; i < 7; i++){
         v[i] = 0x0001 & ((code[w & mask2] & mask) >> i);
         mask <<= 1;
     }

     sprintf(command, "DisplaySevenSegmentDigit %d %d %d %d %d %d %d %d", index, v[0], v[1], v[2], v[3], v[4], v[5], v[6]);

    Tcl_Eval(g_interp, command);

}

/*-----------------------------------------------------------------------*/
void writeMASK(word w)
{
    MASK = w & 0xffff;
}


/*-----------------------------------------------------------------------*/
void refresh(word w)
{
    START = w;
}

/*-----------------------------------------------------------------------*/
void writeTIMER(char control, word w)
{

#ifndef _WIN32
    count.it_interval.tv_sec = 0;
    count.it_value.tv_sec =  0;
#endif

    if(control){
        TIME = w;
	} else {
        if(w & 0x1){
			/** Activar o timer */

#ifdef _WIN32   /* Windows version */
			if(windowsTimerID == 0) {
				previousTime = GetTickCount();
				windowsTimerID = SetTimer(NULL, 0, TIMER_INTERVAL_MILI, decrement);
			}
			/* ELSE: continua como estava */

#else   /* Unix version */
            count.it_interval.tv_usec = TIMER_INTERVAL_MILI * 1000;
            count.it_value.tv_usec = TIMER_INTERVAL_MILI * 1000;
            signal(SIGALRM, decrement);
            setitimer(ITIMER_REAL, &count,0);
#endif

		} else{
			/* Desactivar o timer */
#ifdef _WIN32   /* Windows version */
			KillTimer(NULL, windowsTimerID);
			windowsTimerID = 0;
#else   /* Unix version */
            count.it_interval.tv_usec = 0;
            count.it_value.tv_usec = 0;
			signal(SIGALRM, nothing);
	    	setitimer(ITIMER_REAL, &count, 0);
#endif
		}
    }
}



/*-----------------------------------------------------------------------*/
int uiContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    ui_content(interp->result);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int ivadContent(ClientData clientData, Tcl_Interp *interp,int argc, const char *argv[])
{
    int ivec;

    if(argc != 2)
        return TCL_ERROR;

    ivec = atoi(argv[1]);
    if((ivec < 0) || (ivec > 14)){
        sprintf(interp->result, "Invalid interrupt vector: %s", argv[1]);
        return TCL_ERROR;
    }

    sprintf(interp->result, "%4.4hx", ivad[ivec]);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int intContent(ClientData clientData, Tcl_Interp *interp,int argc, const char *argv[])
{
    int_content(interp->result);

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int mapROMContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int addr;
    int max_addr;
    ctrlROM_addr (*read_mapping_ROM)(word);

    if(argc != 3)
        return TCL_ERROR;

    addr = atoi(argv[1]);
    switch(argv[2][0]){
      case 'A':
	max_addr = N_POS_ROM_MAP_A;
	read_mapping_ROM = read_ROMA;
	break;
      case 'B':
	max_addr = N_POS_ROM_MAP_B;
	read_mapping_ROM = read_ROMB;
	break;
      default:
        sprintf(interp->result, "Invalid mapping ROM: '%c', must be 'A' or 'B'",
                argv[2][0]);
        return TCL_ERROR;
    }

    if((addr < 0) || (addr > max_addr)){
        sprintf(interp->result, "Invalid map ROM %c address: %s",
		argv[2][0], argv[1]);
        return TCL_ERROR;
    }

    sprintf(interp->result, "%2.2hx", read_mapping_ROM(addr));

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int controlROMContent(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int addr;

    if(argc != 2)
        return TCL_ERROR;

    addr = atoi(argv[1]);
    if((addr < 0) || (addr > N_POS_ROM_CONTROL)){
        sprintf(interp->result, "Invalid control ROM address: %s", argv[1]);
        return TCL_ERROR;
    }

    sprintf(interp->result, "%8.8lx", read_control_ROM(addr));

    return TCL_OK;
}


/*-----------------------------------------------------------------------*/
int setIVADValue(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int ivec,
        addr,
        active;

    if(argc != 4)
        return TCL_ERROR;

    ivec = atoi(argv[1]);
    if((ivec < 0) || (ivec > 14)){
        sprintf(interp->result, "Invalid interrupt vector: %s", argv[1]);
        return TCL_ERROR;
    }

    addr = (int) strtol(argv[2], NULL, 16);
    if((addr < 0) || (addr > MAIN_MEM_SIZE)){
        sprintf(interp->result, "Invalid memory address: %s", argv[2]);
        return TCL_ERROR;
    }

    active = atoi(argv[3]);
    if((active != 0) && (active != 1)){
        sprintf(interp->result, "Invalid active value: %s", argv[3]);
        return TCL_ERROR;
    }

    ivad[ivec] = addr;
    active_iv[ivec] = active;

    return TCL_OK;
}
