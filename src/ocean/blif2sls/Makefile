
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

INCL =	Network.h

LSRC =	lex.h.l
YSRC =	parse.Y
SRCS =	main.C Network.C parse.C
OBJS =	main.o Network.o parse.o

TARGET = blif2sls

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

lex.h : $(LSRC)
	$(LEX)  -o $@ -P$(TARGET) $(LSRC)
parse.C: $(YSRC) lex.h
	#$(YACC) -o $@ -p $(TARGET) --defines=parse.h -v $(YSRC)
	$(YACC) -o $@ -p $(TARGET) $(YSRC)

%.o: %.C $(INCL)
	$(CCC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $^ \
		$(LIBPATH)/libocean.a \
		$(LIBPATH)/libseadif.a

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) lex.h parse.C parse.h parse.output

#EOF
