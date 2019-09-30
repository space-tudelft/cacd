
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../.. -DCONFIG_SPACE2=1

DATE = $(shell date)

OBJS  = subcont.o main.o date.o
BOBJS = bipopair.o bipotile.o devices.o junction.o nodelink.o pnedges.o polnode.o
EOBJS = check.o clrtile.o contedge.o gettech.o latcap.o meshedge.o nodepnt.o recog.o
EOBJS2 = einit.o enumpair.o enumtile.o
LOBJS = elem.o elim.o init.o lump.o node.o out.o
SOBJS = edge.o hier.o info.o input.o slant.o tile.o update.o
SOBJS2 = determ.o getparam.o scan.o sp_main.o

TARGET= space2

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

$(BOBJS): %.o: ../bipolar/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(EOBJS): %.o: ../extract/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(EOBJS2): %.o: extract/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(LOBJS): %.o: lump/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SOBJS): %.o: ../scan/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SOBJS2): %.o: scan/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
subcont.o: ../substr/subcont.c
	$(CC) $(CFLAGS) -c -o $@ $<
main.o date.o: %.o: ../space/%.c
	$(CC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c -o $@ $<

$(TARGET): $(BOBJS) $(EOBJS) $(EOBJS2) $(LOBJS) $(SOBJS) $(SOBJS2) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(BOBJS) $(EOBJS) $(EOBJS2) $(LOBJS) $(SOBJS) $(SOBJS2) $(OBJS) \
		$(LIBPATH)/auxil.a \
		$(LIBPATH)/libddm.a -lm
	$(RM) date.o

clean:; $(RM) $(TARGET) $(OBJS) $(BOBJS) $(EOBJS) $(EOBJS2) $(LOBJS) $(SOBJS) $(SOBJS2)

#EOF
