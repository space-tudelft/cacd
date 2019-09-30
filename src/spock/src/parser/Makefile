
include ../../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../.. -I$(QTLIB)/include -DQT_NO_DEBUG=1

INCL = Parser.h
LSRC = lex.h.L
YSRC = parse.Y
OBJS = Parser.o parse.o

TARGET = libParser.a

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@

%.o: %.cc $(INCL)
	$(CCC) $(CFLAGS) -c $<

parse.cc: $(YSRC) lex.h
	$(YACC) -o $@ $(YSRC)
lex.h: $(LSRC)
	$(LEX) -o $@ $(LSRC)

clean:
	$(RM) $(TARGET) $(OBJS) lex.h parse.cc

#EOF
