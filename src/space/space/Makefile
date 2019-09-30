
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

DATE = $(shell date)

OBJS  = subcont.o main.o date.o
BOBJS = bipopair.o bipotile.o devices.o \
	junction.o nodelink.o pnedges.o polnode.o
EOBJS = check.o clrtile.o contedge.o einit.o enumpair.o enumtile.o \
	gettech.o latcap.o meshedge.o nodepnt.o recog.o
LOBJS = elem.o elim.o init.o lump.o node.o out.o ready.o reduc.o
SOBJS = determ.o edge.o getparam.o hier.o info.o input.o \
	scan.o slant.o sp_main.o tile.o update.o

TARGET = space

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

$(BOBJS): %.o: ../bipolar/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(EOBJS): %.o: ../extract/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(LOBJS): %.o: ../lump/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SOBJS): %.o: ../scan/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
subcont.o: ../substr/subcont.c
	$(CC) $(CFLAGS) -c -o $@ $<
main.o date.o: %.o: ../space/%.c
	$(CC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c -o $@ $<

$(TARGET): $(BOBJS) $(EOBJS) $(LOBJS) $(SOBJS) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(BOBJS) $(EOBJS) $(LOBJS) $(SOBJS) $(OBJS) \
		$(LIBPATH)/auxil.a \
		$(LIBPATH)/libddm.a -lm
	$(RM) date.o

clean:; $(RM) $(TARGET) $(OBJS) $(BOBJS) $(EOBJS) $(LOBJS) $(SOBJS)

#EOF
