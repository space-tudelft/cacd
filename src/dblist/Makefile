
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

OBJS = dblist.o lcircuit.o lscells.o

TARGET = dblist

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
