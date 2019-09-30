
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -DSLS=1 -I../..

INCL = define.h extern.h type.h
LSRC1 = cmd_l.h.l
YSRC1 = cmd_y.y
LSRC2 = procp_l.h.l
YSRC2 = procp_y.y
OBJS  = binfil.o chargesh.o dissip.o evalf.o evalv.o eventl.o \
	findhdum.o futil.o getcmd.o getsig.o initmode.o intrup.o \
	plot.o prsignal.o procpar.o resdiv.o resfile.o \
	simul.o slserr.o timing.o cmd_y.o procp_y.o \
	bsearch.o monit.o deb.o find.o hiername.o vfunc.o
OBJS1 = main.o
OBJS2 = main2.o zdeffunc.o

SLSDIR = $(ICDPATH)/lib/sls
SRCDIR = $(ICDPATH)/share/src

TARGET1 = main.o
TARGET2 = sls

install: $(TARGET1) $(TARGET2)
	if [ ! -d $(SRCDIR) ]; then mkdir $(SRCDIR); fi
	if [ -d $(SRCDIR)/sls ]; then rm -fr $(SRCDIR)/sls; fi
	if [ -d $(SRCDIR)/sls_exp ]; then rm -fr $(SRCDIR)/sls_exp; fi
	mkdir $(SRCDIR)/sls
	mkdir $(SRCDIR)/sls_exp
	cp func.h		 $(SRCDIR)/sls
	cp ../sls_exp/gntype.h   $(SRCDIR)/sls_exp
	cp ../sls_exp/gndefine.h $(SRCDIR)/sls_exp

cmd_l.h: $(LSRC1)
	$(LEX)  -o $@ -Pyy0 $(LSRC1)
cmd_y.c: $(YSRC1) cmd_l.h
	$(YACC) -o $@ -p yy0 $(YSRC1)
procp_l.h: $(LSRC2)
	$(LEX)  -o $@ -Pyy1 $(LSRC2)
procp_y.c: $(YSRC2) procp_l.h
	$(YACC) -o $@ -p yy1 $(YSRC2)
main2.c:
	ln -s main.c $@

%.o: ../sls_exp/%.c $(INCL)
	$(CC) $(CFLAGS) -c $<
%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET1): main.c $(OBJS)
	$(CC) $(CFLAGS) -DFUNCSLS=1 -c main.c
	if [ -d $(SLSDIR) ]; then rm -fr $(SLSDIR); fi
	mkdir $(SLSDIR)
	cp $(OBJS) $(OBJS1) $(SLSDIR)

$(TARGET2): $(OBJS) $(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a
	mv $@ $(BINPATH)

clean:
	$(RM) $(TARGET2) $(OBJS) $(OBJS1) $(OBJS2) main2.c
	$(RM) cmd_l.h cmd_y.c procp_l.h procp_y.c

#EOF
