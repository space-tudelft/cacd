
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../.. -DCONFIG_XSPACE=1

DATE = $(shell date)

OBJS =	gettech.o getparam.o sp_main.o main.o date.o
XOBJS = draw.o exposel.o interact.o menu.o menubox.o menush.o \
	rgb.o robot.o x2d.o x3d.o

TARGET = Xspace
APPDIR = $(ICDPATH)/share/lib/app-defaults

install: $(TARGET)
	mv $(TARGET) $(BINPATH)
	if [ ! -d $(APPDIR) ]; then mkdir $(APPDIR); fi
	rm -f $(APPDIR)/Xspace
	cp ../X11/xspace.res $(APPDIR)/Xspace

$(XOBJS): %.o: ../X11/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
gettech.o: %.o: ../extract/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
getparam.o sp_main.o: %.o: ../scan/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
main.o date.o: %.o: ../space/%.c
	$(CC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c -o $@ $<

$(TARGET): $(XOBJS) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(XOBJS) $(OBJS) \
		-lXaw -lXt -lX11 \
		$(LIBPATH)/auxil.a \
		$(LIBPATH)/libddm.a -lm
	$(RM) date.o

clean:; $(RM) $(TARGET) $(OBJS) $(XOBJS)

#EOF
