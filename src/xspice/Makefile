
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL =	define.h extern.h incl.h model.h type.h
LSRC =	lex.l
YSRC =	parse.y
OBJS =	cirTree.o eqnHand.o findFunc.o findNet.o findNetw.o nextAttr.o nodetab.o lex.o xnetwork.o
OBJS1 = ctrlFile1.o findDev.o main.o  model1.o parse.o prFoot1.o prHead1.o prInst1.o prNets1.o print1.o
OBJS2 = ctrlFile.o findDev2.o main2.o model.o parse2.o prFoot.o  prHead.o prInst.o prNets.o print.o
OBJS3 = ctrlFile.o findDev3.o main3.o model.o parse.o  prFoot3.o prHead3.o prInst3.o prNets3.o print3.o

TARGET1= xspectre
TARGET2= xspice
TARGET3= xpstar

install:   $(TARGET1) $(TARGET2) $(TARGET3)
	mv $(TARGET1) $(TARGET2) $(TARGET3) $(BINPATH)

lex.c: $(LSRC)
	$(LEX)  -o $@ $(LSRC)
parse.c: $(YSRC)
	$(YACC) -o $@ --defines=parse.h $(YSRC)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<
ctrlFile1.o: ctrlFile.c $(INCL)
	$(CC) $(CFLAGS) -DXSPECTRE=1 -c $< -o $@
model1.o: model.c $(INCL)
	$(CC) $(CFLAGS) -DXSPECTRE=1 -c $< -o $@
parse2.o: parse.c $(INCL)
	$(CC) $(CFLAGS) -DXSPICE=1 -c $< -o $@
findDev.o: findDev.c $(INCL)
	$(CC) $(CFLAGS) -DXSPECTRE=1 -c $< -o $@
findDev2.o: findDev.c $(INCL)
	$(CC) $(CFLAGS) -DXSPICE=1 -c $< -o $@
findDev3.o: findDev.c $(INCL)
	$(CC) $(CFLAGS) -DXPSTAR=1 -c $< -o $@
main.o: main.c $(INCL)
	$(CC) $(CFLAGS) -DXSPECTRE=1 -c $< -o $@
main2.o: main.c $(INCL)
	$(CC) $(CFLAGS) -DXSPICE=1 -c $< -o $@
main3.o: main.c $(INCL)
	$(CC) $(CFLAGS) -DXPSTAR=1 -c $< -o $@
prFoot1.o: prFoot.c $(INCL)
	$(CC) $(CFLAGS) -DXSPECTRE=1 -c $< -o $@
prFoot3.o: prFoot.c $(INCL)
	$(CC) $(CFLAGS) -DXPSTAR=1 -c $< -o $@
prHead1.o: prHead.c $(INCL)
	$(CC) $(CFLAGS) -DXSPECTRE=1 -c $< -o $@
prHead3.o: prHead.c $(INCL)
	$(CC) $(CFLAGS) -DXPSTAR=1 -c $< -o $@
prInst1.o: prInst.c $(INCL)
	$(CC) $(CFLAGS) -DXSPECTRE=1 -c $< -o $@
prInst3.o: prInst.c $(INCL)
	$(CC) $(CFLAGS) -DXPSTAR=1 -c $< -o $@
prNets1.o: prNets.c $(INCL)
	$(CC) $(CFLAGS) -DXSPECTRE=1 -c $< -o $@
prNets3.o: prNets.c $(INCL)
	$(CC) $(CFLAGS) -DXPSTAR=1 -c $< -o $@
print1.o: print.c $(INCL)
	$(CC) $(CFLAGS) -DXSPECTRE=1 -c $< -o $@
print3.o: print.c $(INCL)
	$(CC) $(CFLAGS) -DXPSTAR=1 -c $< -o $@

$(TARGET1): $(OBJS1) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm
$(TARGET2): $(OBJS2) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm
$(TARGET3): $(OBJS3) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm

clean:
	$(RM) $(TARGET1) $(TARGET2) $(TARGET3) $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS)
	$(RM) lex.c parse.c parse.h

#EOF
