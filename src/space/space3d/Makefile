
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../.. -DCONFIG_SPACE3D=1

DATE = $(shell date)

OBJS  = draw.o interact.o subcont.o spedge.o main.o date.o
BOBJS = bipopair.o bipotile.o devices.o \
	junction.o nodelink.o pnedges.o polnode.o
EOBJS = check.o clrtile.o contedge.o einit.o enumpair.o enumtile.o \
	gettech.o latcap.o meshedge.o nodepnt.o recog.o
LOBJS = elem.o elim.o init.o lump.o node.o out.o ready.o reduc.o
SOBJS = determ.o edge.o getparam.o hier.o info.o input.o \
	scan.o slant.o sp_main.o tile.o update.o
VOBJS = cap3d.o convex.o displace.o dogreen.o face.o \
	matherr.o mesh.o pqueue.o recogm.o refine.o sphash.o \
	spider.o sppair.o sptile.o sputil.o strip.o triang.o

TARGET = space3d

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
$(VOBJS): %.o: ../spider/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
spedge.o: ../spider/edge.c
	$(CC) $(CFLAGS) -c -o $@ $<
subcont.o: ../substr/subcont.c
	$(CC) $(CFLAGS) -c -o $@ $<
draw.o interact.o: %.o: ../X11/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
main.o date.o: %.o: ../space/%.c
	$(CC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c -o $@ $<

$(TARGET): $(BOBJS) $(EOBJS) $(LOBJS) $(SOBJS) $(VOBJS) $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $(BOBJS) $(EOBJS) $(LOBJS) $(SOBJS) $(VOBJS) $(OBJS) \
		$(LIBPATH)/schur.a \
		$(LIBPATH)/green.a \
		$(LIBPATH)/auxil.a \
		$(LIBPATH)/libddm.a -lm
	$(RM) date.o

clean:; $(RM) $(TARGET) $(OBJS) $(BOBJS) $(EOBJS) $(LOBJS) $(SOBJS) $(VOBJS)

#EOF
