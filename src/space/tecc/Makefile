
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

INCL =	define.h extern.h type.h
LSRC =	lex.L
YSRC =	parse.Y
OBJS =	change.o check.o debug.o findcond.o \
	main.o mkcapbit.o mkkeys.o parse.o lex.o \
	prtabs.o selkeys.o tclexport.o

TARGET = tecc

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

parse.cc: $(YSRC)
	$(YACC) -o parse.cc --defines=parse.h $(YSRC)
lex.cc: $(LSRC) parse.cc
	$(LEX) -o lex.cc $(LSRC)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

%.o: %.cc $(INCL)
	$(CCC) $(CFLAGS) -c $<

%.o: %.C $(INCL)
	$(CCC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $(OBJS) \
		$(LIBPATH)/auxil.a \
		$(LIBPATH)/libddm.a \
		$(LIBPATH)/libmin.a

clean:; $(RM) $(TARGET) $(OBJS) lex.cc parse.cc parse.h

#EOF
