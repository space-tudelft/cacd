
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL = extern.h incl.h main.h
LSRC = cmsk_lex.l
YSRC = cmsk_parse.y
OBJS = main.o edge.o func.o pat.o proc_text.o cmsk_parse.o cmsk_lex.o

TARGET = cmsk

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

cmsk_lex.c: $(LSRC) cmsk_parse.h
	$(LEX) -o cmsk_lex.c $(LSRC)
cmsk_parse.c: $(YSRC)
	$(YACC) -o cmsk_parse.c --defines=cmsk_parse.h $(YSRC)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm

clean:
	$(RM) $(TARGET) $(OBJS)
	$(RM) cmsk_lex.c cmsk_parse.c cmsk_parse.h

#EOF
