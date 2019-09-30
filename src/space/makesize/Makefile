
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS =	extract.o gettech.o input.o main.o makesize.o output.o \
	redge.o rscan.o rslant.o tile.o update.o \
	../makegln/sort.o \
	../makegln/slant.o \
	../makegln/scan.o \
	../makegln/gln.o \
	../makegln/edge.o \
	../makegln/pqueue.o \
	../makegln/qsort.o

TARGET= makesize

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

gettech.o: ../extract/gettech.c
	$(CC) $(CFLAGS) -DMAKESIZE -c $<
output.o: ../makegln/output.c
	$(CC) $(CFLAGS) -DMAKESIZE -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/auxil.a $(LIBPATH)/libddm.a -lm

clean:
	$(RM) *.o $(TARGET)

#EOF
