
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL =	define.h extern.h incl.h model.h type.h
LSRC =	lex.l
YSRC =	parse.y
OBJS =	ctrlFile.o cirTree.o eqnHand.o findDev.o findFunc.o findNet.o findNetw.o \
	main.o model.o nextAttr.o nodetab.o parse.o lex.o \
	prFoot.o prHead.o prInst.o prNets.o print.o xnetwork.o

TARGET = xspf
TARGET2= xspef

install: $(TARGET)
	ln -s $(TARGET) $(TARGET2)
	mv $(TARGET) $(TARGET2) $(BINPATH)

lex.c: $(LSRC)
	$(LEX)  -o $@ $(LSRC)
parse.c: $(YSRC)
	$(YACC) -o $@ --defines=parse.h $(YSRC)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm

clean:
	$(RM) $(TARGET) $(OBJS) lex.c parse.c parse.h

#EOF
