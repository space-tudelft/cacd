
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

LSRC =	sdflex.h.l
YSRC =	sdfparse.y

OBJS =	sdfparse.o alias.o defaultenv.o delete.o dumpdb.o err.o hashstring.o imagedesc.o \
	libnametoptr.o memman.o namelist.o readdb.o sdfreport.o seaflat.o sealibio.o \
	signal.o slicecleanup.o sm.o stv_table.o support.o symlink.o thedate.o timecvt.o

TARGET = libseadif.a

install : $(TARGET)
	mv $(TARGET) $(LIBPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

sdflex.h : $(LSRC)
	$(LEX)  -o $@ $(LSRC)
sdfparse.c: $(YSRC) sdflex.h
	#$(YACC) -o $@ --defines=sdfparse.h -v $(YSRC)
	$(YACC) -o $@ $(YSRC)

$(TARGET): $(OBJS)
	ar crs $@ $^
	ranlib $@

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) sdflex.h sdfparse.c sdfparse.h sdfparse.output

#EOF
