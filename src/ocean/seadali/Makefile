
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS =	add_poly.o add_quad.o add_wire.o animate.o array.o \
	bintree.o bound.o buffer.o check.o clip.o command.o \
	comment.o del_quad.o disp_mask.o display.o distance.o \
	dsgn_dat.o edge_t_ch.o expand.o ext_check.o fish.o \
	get.o global.o grid.o inform.o input.o insert.o \
	inst_draw.o main.o mask.o memo.o menu.o mod.o modl.o \
	mpar.o output.o picture.o poly_trap.o precheck.o \
	q_build.o q_clear.o q_delete.o q_insert.o q_search.o \
	real_main.o s_o_g.o screen.o sngrid.o subr.o term.o \
	termbuf.o tlift.o tpar.o window.o wire.o

TARGET = seadali

install: $(TARGET)
	mv $(TARGET) $(BINPATH)
	if [ ! -d $(ICDPATH)/share/lib/seadali ]; then mkdir $(ICDPATH)/share/lib/seadali; fi
	rm -f $(ICDPATH)/share/lib/seadali/help.html
	cp help.html $(ICDPATH)/share/lib/seadali

%.o: %.c
	$(CC) $(CFLAGS) -c $<
%.o: %.C
	$(CCC) $(CFLAGS) -c $<
screen.o: screen.c
	$(CC) $(CFLAGS) -DCACDCMAP=1 -c $<

$(TARGET): $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $^ \
		$(LIBPATH)/libcmap.a -lX11 \
		$(LIBPATH)/libnelsea.a \
		$(LIBPATH)/libddm.a \
		$(LIBPATH)/libocean.a \
		$(LIBPATH)/libseadif.a

clean:
	$(RM) $(OBJS) $(TARGET)

#EOF
