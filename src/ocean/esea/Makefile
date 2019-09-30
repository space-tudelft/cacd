
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

DATE = $(shell date)

LSRC =	edif.l
YSRC =	edif.y
OBJS =	edifl.o edify.o globals.o main.o nelsisTarget.o \
	report.o seadifTarget.o solveRef.o thedate.o

TARGET = esea

install : $(TARGET)
	ln -s $(TARGET) cedif
	mv cedif $(TARGET) $(BINPATH)

edifl.c: $(LSRC) edify.c
	$(LEX)  -o $@ -Pedif $(LSRC)
edify.c: $(YSRC)
	#$(YACC) -o $@ -p edif --defines=edif.h -v $(YSRC)
	$(YACC) -o $@ -p edif --defines=edif.h $(YSRC)
%.o: %.c
	$(CC) $(CFLAGS) -c $<
thedate.o: thedate.c
	$(CC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c $<

$(TARGET): $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $^ \
		$(LIBPATH)/libddm.a \
		$(LIBPATH)/libocean.a \
		$(LIBPATH)/libseadif.a
	$(RM) thedate.o

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) edifl.c edify.c edify.output edif.h

#EOF
