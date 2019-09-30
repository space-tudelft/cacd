
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

OBJS = main.o mktree.o

TARGET = exp

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(OBJS)

#END
