
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL =	class.h mkdbdefs.h mkdbincl.h slserr.h sys_incl.h
CINCL=	odb.h dict.h netelem.h ntwdefs.h queue.h stack.h xelem.h
CSRC =	diction.C    netelem.C ntwdefs.C queue.C stack.C xelem.C
LSRC =	lex.L
LSRC2=	spilex.L
YSRC =	parse.Y
YSRC2=	spipar.Y
OBJS =	sig_init.o class.o dbopen.o dbaccess.o \
	neteqv.o init_ntw.o simple.o end_ntw.o \
	to_db.o slserr.o util.o bifunc.o dffunc.o
OBJS1=	main.o parse.o lex.o
OBJS2=	spimain.o spipar.o spilex.o

TARGET1= csls
TARGET2= cspice

install: $(TARGET1) $(TARGET2)
	mv $(TARGET1) $(TARGET2) $(BINPATH)

lex.C: $(LSRC) parse.h
	$(LEX) -o lex.C $(LSRC)
parse.C: $(YSRC)
	$(YACC) -o parse.C --defines=parse.h $(YSRC)
spilex.C: $(LSRC2) spipar.h
	$(LEX) -o spilex.C $(LSRC2)
spipar.C: $(YSRC2)
	$(YACC) -o spipar.C --defines=spipar.h $(YSRC2)
spimain.C:
	ln -s main.C $@

%.o: %.C $(INCL)
	$(CCC) $(CFLAGS) -c $<
class.o: class.C $(CSRC) $(INCL) $(CINCL)
	$(CCC) $(CFLAGS) -c $<
spimain.o: spimain.C $(INCL)
	$(CCC) $(CFLAGS) -DSPICE=1 -c $<

$(TARGET1): $(OBJS) $(OBJS1)
	$(CCC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a
$(TARGET2): $(OBJS) $(OBJS2)
	$(CCC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET1) $(TARGET2) $(OBJS) $(OBJS1) $(OBJS2) spimain.C
	$(RM) spilex.C spipar.C spipar.h
	$(RM) lex.C parse.C parse.h

#EOF
