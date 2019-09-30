
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL = func_parsedefs.h
LSRC = lex.l
YSRC = func_parse.y
OBJS = main.o func_parse.o lex.o addfunte.o addfunob.o parsefuncs.o rout_evals.o

TARGET = cfun

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

lex.c: $(LSRC)
	$(LEX) -o lex.c $(LSRC)
func_parse.c: $(YSRC)
	$(YACC) -o func_parse.c --defines=func_parse.h $(YSRC)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<
lex.o: lex.c $(INCL) func_parse.h
	$(CC) $(CFLAGS) -DYYDEBUG=1 -c $<
func_parse.o: func_parse.c $(INCL)
	$(CC) $(CFLAGS) -DYYDEBUG=1 -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(OBJS)
	$(RM) lex.c func_parse.c func_parse.h

#EOF
