CC=gcc
YFLAGS	= -dvy
YACC 	= bison
#CFLAGS	= -ansi -g -O2 -DDEBUG
 CFLAGS	= -O2 -ansi
LEX	= flex -i
#LEX	= flex -i, %option noyywrap
P	= main.o assem.o labels.o scan.o

all:	  	p3as
p3as: 		$(P)
		$(CC) -o p3as $(P) -lfl
mod_date.h:	main.c assem.y assem.h scan.l labels.c
	        date +'"%d/%m/%y, %T"' > mod_date.h
assem.c:	assem.y
assem.o: 	assem.c assem.h
main.o:		main.c assem.h mod_date.h
labels.o:	labels.c
scan.o:		scan.l

clean:
		rm *.o y.output assem.c p3as
