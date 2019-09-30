
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS =	gettech.o main.o output.o scan.o \
	../makegln/edge.o \
	../makegln/slant.o \
	../makegln/sort.o \
	../makegln/qsort.o \
	../makegln/pqueue.o \
	../makemesh/input.o

TARGET= makedela

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

gettech.o: ../extract/gettech.c
	$(CC) $(CFLAGS) -DMAKEDELA -c $<
main.o: ../makegln/main.c
	$(CC) $(CFLAGS) -DMAKEDELA -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/auxil.a $(LIBPATH)/libddm.a -lm

clean:
	$(RM) *.o $(TARGET)

#EOF
