
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL =	define.h extern.h

OBJS =	main.o auxil.o errexit.o expbox.o expimage.o expmodel.o \
	readbox.o readnor.o readterm.o readhtm.o sexp_box.o trmctree.o

TARGET = makeboxl

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm

clean:
	$(RM) *.o

#END
