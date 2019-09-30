
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL = header.h
OBJS =	add_poly.o add_quad.o add_wire.o animate.o array.o bintree.o \
	bound.o buffer.o check.o clip.o dalirun.o del_quad.o disp_mask.o \
	display.o distance.o dsgn_dat.o edge_t_ch.o expand.o ext_check.o \
	grid.o inform.o input.o insert.o inst_draw.o memo.o mod.o modl.o \
	mpar.o output.o poly_trap.o precheck.o q_build.o q_clear.o \
	q_delete.o q_insert.o q_search.o sngrid.o term.o termbuf.o \
	tlift.o tpar.o wire.o global.o command.o get.o main.o mask.o \
	menu.o picture.o screen.o subr.o window.o comment.o

TARGET = dali

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ \
			$(LIBPATH)/libcmap.a -lX11 \
			$(LIBPATH)/libddm.a -lm

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
