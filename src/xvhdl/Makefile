
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL =	define.h extern.h incl.h model.h type.h

OBJS =	ctrlFile.o cirTree.o findDev.o findFunc.o findNet.o findNetw.o main.o \
	nextAttr.o prExt.o prFoot.o prHead.o prInst.o prNets.o print.o xnetwork.o

TARGET = xvhdl

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
