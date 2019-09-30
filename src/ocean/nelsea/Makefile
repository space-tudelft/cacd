
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

LSRC =	imagelex.h.l
YSRC =	imageparse.y
OBJS =	imageparse.o alloc.o error.o globals.o grid_globals.o \
	tin_table.o csi_table.o hostname.o initialize.o makelib.o \
	mapfile.o nelsis.o powerFix.o read_cell.o read_circuit.o \
	read_layout.o read_seadif.o sea.o writ_circuit.o write_layout.o write_seadif.o \
	seadifGraph.o ghotiDelete.o ghoti.o serPar.o runprog.o
OBJ1 =	main.o

TARGET = libnelsea.a
TARGET1= nelsea
TARGET2= fish
TARGET3= ghoti
TARGET4= sea

install : $(TARGET) $(TARGET1)
	ln -s $(TARGET1) $(TARGET2)
	ln -s $(TARGET1) $(TARGET3)
	ln -s $(TARGET1) $(TARGET4)
	mv $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(BINPATH)

imagelex.h: $(LSRC)
	$(LEX)  -o $@ -Pyd $(LSRC)
imageparse.c: $(YSRC) imagelex.h
	#$(YACC) -o $@ -p yd --defines=imageparse.h $(YSRC)
	$(YACC) -o $@ -p yd $(YSRC)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
%.o: %.C
	$(CCC) $(CFLAGS) -c $<
main.o: main.C
	$(CCC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c $<

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@
	mv $@ $(LIBPATH)

$(TARGET1): $(OBJ1)
	$(CCC) $(LDFLAGS) -o $@ $(OBJ1) \
			$(LIBPATH)/libnelsea.a \
			$(LIBPATH)/libddm.a \
			$(LIBPATH)/libocean.a \
			$(LIBPATH)/libseadif.a

clean:
	$(RM) $(OBJS) $(OBJ1) $(TARGET) $(TARGET1)
	$(RM) imagelex.h imageparse.c imageparse.h

#EOF
