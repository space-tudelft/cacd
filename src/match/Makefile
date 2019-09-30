
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL =	head.h proto.h incl.h macro.h mod.h type.h
LSRC =	lex.h.l
YSRC =	yacc.y
OBJS =	access.o auxilary.o block.o cirTree.o color.o \
	debug.o error.o hash.o init.o link.o \
	main.o map.o mark.o network.o opt.o \
	param.o prime.o print.o queue.o \
	read.o readdb.o readDev.o \
	stack.o symbol.o utils.o xnetw.o yacc.o

TARGET = match

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

lex.h : $(LSRC)
	$(LEX)  -o $@ $(LSRC)
yacc.c: $(YSRC) lex.h
	$(YACC) -o $@ $(YSRC)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libese.a $(LIBPATH)/libddm.a

s_map: s_map.c head.h
	$(CC) -o $@ $(CFLAGS) s_map.c

clean:
	$(RM) *.o s_map lex.h yacc.c

#EOF
