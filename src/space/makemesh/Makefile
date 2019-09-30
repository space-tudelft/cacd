
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS =	gln.o input.o main.o output.o scan.o sort.o \
	../makegln/edge.o \
	../makegln/pqueue.o \
	../makegln/qsort.o \
	../makegln/slant.o

LIBS =	$(LIBPATH)/auxil.a $(LIBPATH)/libddm.a

TARGET= makemesh

install: $(TARGET) mkmesh
	mv $(TARGET) mkmesh $(BINPATH)

main.o: ../makegln/main.c
	$(CC) $(CFLAGS) -DMAKEMESH -c $<
output.o: ../makegln/output.c
	$(CC) $(CFLAGS) -DMAKEMESH -c $<
scan.o: ../makegln/scan.c
	$(CC) $(CFLAGS) -DMAKEMESH -c $<
sort.o: ../makegln/sort.c
	$(CC) $(CFLAGS) -DMAKEMESH -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

mkmesh: mkmesh.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

wrmesh: wrmesh.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) *.o $(TARGET) mkmesh wrmesh

#EOF
