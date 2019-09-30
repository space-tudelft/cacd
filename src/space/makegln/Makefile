
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

INCL =	config.h makegln.h proto.h

OBJS =	edge.o gln.o input.o \
	main.o output.o pqueue.o \
	qsort.o scan.o slant.o sort.o

TARGET= makegln

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/auxil.a $(LIBPATH)/libddm.a

clean:
	$(RM) *.o $(TARGET)

#EOF
