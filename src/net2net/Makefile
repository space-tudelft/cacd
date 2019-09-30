
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL =	incl.h

OBJS =	findNet.o main.o prInst.o xnetwork.o

TARGET = net2net

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
