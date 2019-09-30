
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

DATE = $(shell date)

LSRC =	imagelex.h.l
YSRC =	imageparse.y
OBJS =	imageparse.o alloc.o build.o error.o finish_up.o globals.o gridpoint.o \
	lee.o mk_datastr.o power_route.o print_grid.o read_seadif.o \
	route.o seadif_error.o write_seadif.o main.o thedate.o

TARGET = trout

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
thedate.o: thedate.c
	$(CC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c $<

imagelex.h : $(LSRC)
	$(LEX)  -Pyd -o $@ $(LSRC)
imageparse.c : $(YSRC) imagelex.h
	#$(YACC) -p yd -o $@ --defines=imageparse.h $(YSRC)
	$(YACC) -p yd -o $@ $(YSRC)

$(TARGET): $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $^ \
		$(LIBPATH)/libocean.a \
		$(LIBPATH)/libseadif.a
	$(RM) thedate.o

clean:
	$(RM) $(OBJS) $(TARGET) imagelex.h imageparse.c imageparse.h

#EOF
