
include ../../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../..

OBJS =	clusterPerm.o findrect.o madonna.o matrixInt.o globRoute.o matrixGrid.o \
	alarm.o cost.o dopart.o genpart.o madonna_.o randomMove.o

TARGET = libpartitioner.a

install : $(TARGET)
	mv $(TARGET) $(LIBPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
%.o: %.C
	$(CCC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	ar crs $@ $^
	ranlib $@

clean:
	$(RM) $(OBJS) $(TARGET)

#EOF
