/*-------------------------------------------------------------------------
| Simulator for the CISC uP described in Chapter 12 of
| "Introducao aos Sistemas Digitais e Microprocessadores",
| by G. Arroz, J. Monteiro and A. Oliveira,
| IST Press, 2003
|
| Definitions for the simulator.
|
| History: 17/mar/01 - JCM - created.
|
| Copyright (c) 2001	Guilherme Arroz, egsa@alfa.ist.utl.pt
|                       Jose' Monteiro, jcm@inesc.pt
|                 	Arlindo Oliveira, aml@inesc.pt
|                       Technical University of Lisbon, Portugal
+------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <signal.h>
#endif

/* Data types */
typedef unsigned long dword;             /* 32-bit word for microinstructions */
typedef unsigned short int word;         /* data size of instruction/registers*/
typedef unsigned char ivad_index;        /* index for interrupt vector */
typedef unsigned short int mem_addr;     /* main memory address */
typedef unsigned int ctrlROM_addr;       /* control ROM address */

typedef void (*sighandler_t)(int);

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define NIL(type) ((type *) 0)
#define FREE(obj) ((obj) ? (free((char *) (obj)), (obj) = 0) : 0)


/* Flags indexes */
#define INT_EN 4
#define ZERO 3
#define CARRY 2
#define NEGATIVE 1
#define OVERFLOW 0
#define uZERO 3
#define uCARRY 2
#define uNEGATIVE 1
#define uOVERFLOW 0

/* Special registers */
#define PC_R15 15
#define SP_R14 14

/* Main memory */
#define MAIN_MEM_SIZE (1 << (sizeof(mem_addr)*8))

/* Address where IO ports start, inclusive */
#define IO_ADDR 0xff00

/* Control ROM */
#define N_POS_ROM_CONTROL 512

/* Mapping ROMs */
#define N_POS_ROM_MAP_A 64
#define N_POS_ROM_MAP_B 16

/* Number of IVADs */
#define N_IVAD 16        /* Should be 256, but we only use 16... */


/* Random constants */
#define LINE_LENGTH 128
#define P3AS_MARK 56347222

