
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS = input.o main.o scan.o plot.o

TARGET = mplot

install: $(TARGET)
	mv $(TARGET) $(BINPATH)
	rm -f $(ICDPATH)/share/lib/epslay.pro
	cp epslay.pro $(ICDPATH)/share/lib

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/auxil.a $(LIBPATH)/libddm.a

clean:; $(RM) *.o $(TARGET)

#EOF
