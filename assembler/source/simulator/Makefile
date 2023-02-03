CC        = gcc

P         = p3SimTcl
PSRC      = main.c commands.c mem.c microcode.c mapRom.c control.c datapath.c unassembler.c
POBJ     := $(PSRC:.c=.o)

COMMONH   = common.h
PHDR      = defs_sim.h defs_inter.h $(COMMONH)

LIBS      = -ltcl -ltk -lm
CFLAGS    = -pipe
LDFLAGS   = $(LIBS)


LINTFLAGS =  $(INCLUDE) ${LINTEXTRAS}
LINTLIBS  =




.PHONY:all
all:	$(P)

$(P):	$(POBJ)
	@echo [LD] $(POBJ) TO $(P)
	@$(CC) $(LDFLAGS) -o $(P) $(POBJ) $(LIBS) $(CFLAGS)

%.o:	%.c
	@echo [CC] $< TO $@
	@$(CC) $(CFLAGS) -c $< -o $@


main.o:			$(COMMONH) defs_inter.h
commands.o:		$(COMMONH) defs_inter.h
mem.o:			$(COMMONH) defs_sim.h
microcode.o:	$(COMMONH) defs_sim.h
mapRom.o:		$(COMMONH) defs_sim.h
control.o:		$(COMMONH) defs_sim.h
datapath.o:		$(COMMONH) defs_sim.h
unassembler.o:	$(COMMONH) defs_sim.h

.PHONY:clean
clean:
	@echo [RM] $(P) $(POBJ)
	@rm -f $(P) $(POBJ)



.PHONY:lint
lint:	$(PSRC) $(PHDR)
	lint $(LINTFLAGS) $(PSRC) $(LINTLIBS)
