
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL =	define.h extern.h

OBJS =	aux2.o errexit.o exp_box.o exp_model.o main.o mak_actreg.o \
	read_box.o read_mc.o read_term.o tra_mctree.o upd_motree.o

TARGET = makeboxh

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
