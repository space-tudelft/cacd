
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

SRCDIR = $(ICDPATH)/share/src

TARGET = libddm.a

HDRS = dmdebug.h dmdef.h dmfuncs.h dmglobal.h dmincl.h dmproto.h dmstd.h

OBJS = dmaddimp.o dmatprnt.o dmatscan.o dmcelleq.o dmcellst.o \
	dmchkin.o dmchkout.o dmclpcnt.o dmclproj.o dmcstrm.o \
	dmdata.o dmdebug.o dmdoget.o dmdoput.o dmerror2.o dmerror.o \
	dmexist.o dmfatal.o dmfather.o dmfproj.o dmgcstat.o \
	dmgdata.o dmgetesc.o dmgproc.o dmifdeb.o dmimlst.o \
	dminit.o dmisroot.o dmkey.o dmlockpr.o dmmeta.o \
	dmopproj.o dmostrm.o dmoxdata.o dmpack.o dmpcstat.o \
	dmpdata.o dmperror.o dmprkey.o dmprlst.o dmprpth.o \
	dmputesc.o dmrmcell.o dmrstrm.o dmrun.o dmseek.o dmsprtf.o \
	dmstat.o dmstring.o dmtell.o dmtname.o dmunlink.o dmutil.o

install: $(TARGET)
	mv $(TARGET) $(LIBPATH)
	if [ ! -d $(SRCDIR) ]; then mkdir $(SRCDIR); fi
	if [ -d $(SRCDIR)/libddm ]; then rm -fr $(SRCDIR)/libddm; fi
	mkdir $(SRCDIR)/libddm
	cp $(HDRS) $(SRCDIR)/libddm

$(TARGET): $(OBJS)
	ar crs $(TARGET) $(OBJS)

dmrun: dmrun.c
	$(CC) $(CFLAGS) -DDRIVER -o $@ $< $(LIBPATH)/libddm.a

clean:
	rm -f $(OBJS) $(TARGET) dmrun

#EOF
