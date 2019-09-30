
include ../../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../..

LSRC =	imagelex.h.l
YSRC =	imageparse.Y
OBJS =	imageparse.o clst.o cluster.o compact.o cri.o divTable.o \
	imageMap.o layMap.o parserIf.o pattern.o phil.o plane.o \
	plcm.o plcminit.o point.o protArea.o usrlib.o window.o \
	debug.o mtx.o stuff.o initimagedsc.o

TARGET = libphil.a

install: $(TARGET)
	mv $(TARGET) $(LIBPATH)

imagelex.h: $(LSRC)
	$(LEX)  -Ppp -o $@ $(LSRC)
imageparse.C: $(YSRC) imagelex.h
	#$(YACC) -p pp -o $@ --defines=imageparse.h $(YSRC)
	$(YACC) -p pp -o $@ $(YSRC)

%.o: %.C
	$(CCC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	ar crs $@ $^
	ranlib $@

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) imagelex.h imageparse.C imageparse.h

#EOF
