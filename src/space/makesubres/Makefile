
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../.. -DCONFIG_SPACE3D

DATE = $(shell date)

OBJS =	edge.o enumtile.o face.o gettech.o input.o lump.o mesh.o pqueue.o refine.o scan.o slant.o \
	spedge.o spider.o sppair.o sptile.o strip.o sub3d.o subcont.o tile.o update.o main.o

TARGET1= makesubres
TARGET2= makesubcap

install: $(TARGET1)
	mv $(TARGET1) $(TARGET2) $(BINPATH)

gettech.o: ../extract/gettech.c
	$(CC) $(CFLAGS) -DMAKEDELA -c -o $@ $<
main.o: main.c
	$(CC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c -o $@ $<

$(TARGET1): $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $(OBJS) \
			$(LIBPATH)/schur.a \
			$(LIBPATH)/green.a \
			$(LIBPATH)/auxil.a \
			$(LIBPATH)/libddm.a -lm
	@$(RM) main.o $(TARGET2)
	ln -s $(TARGET1) $(TARGET2)

clean:
	$(RM) *.o $(TARGET1) $(TARGET2)

#EOF
