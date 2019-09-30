
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL = elements.h externs.h option.h realist.h xmrealist.h xdclass.h

SRCS = README.helios helios.icon helios.message sram.xpm
SRC2 = helios.res

OBJS = realist.o xmrealist.o run_space.o adminvar.o get_db.o callbacks.o \
	glue.o xdxmdialog.o xdxtclass.o

TARGET = helios
DIR1 = $(ICDPATH)/share/lib/helios
DIR2 = $(ICDPATH)/share/lib/app-defaults

install: $(TARGET)
	mv $(TARGET) $(BINPATH)
	rm -fr $(DIR1)
	mkdir $(DIR1)
	cp $(SRCS) $(DIR1)
	if [ ! -d $(DIR2) ]; then mkdir $(DIR2); fi
	rm -f $(DIR2)/Helios
	cp $(SRC2) $(DIR2)/Helios

adminvar.C:	realist.h externs.h option.h xmrealist.h xdclass.h
callbacks.C:	realist.h externs.h option.h xmrealist.h xdclass.h
get_db.C:	realist.h externs.h
glue.C:		elements.h
realist.C:	realist.h option.h xmrealist.h xdclass.h
run_space.C:	realist.h externs.h
xmrealist.C:	option.h xmrealist.h xdclass.h
xdxmdialog.C:	xdclass.h
xdxtclass.C:	xdclass.h

%.o: %.C
	$(CCC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $^ \
		$(LIBPATH)/auxil.a \
		-lXm -lXt -lX11 \
		$(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
#	$(LIBPATH)/libxdclass.a
#EOF
