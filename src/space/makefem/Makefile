
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS =	edge.o enumtile.o gettech.o input.o main.o scan.o slant.o tile.o update.o

TARGET= makefem

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

gettech.o: ../extract/gettech.c
	$(CC) $(CFLAGS) -DMAKEDELA -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPATH)/auxil.a $(LIBPATH)/libddm.a

clean:
	$(RM) *.o $(TARGET)

#EOF
