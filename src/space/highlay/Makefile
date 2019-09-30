
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS =	main.o rmatch.o

TARGET= highlay

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/auxil.a $(LIBPATH)/libddm.a

clean:
	$(RM) *.o $(TARGET)

#EOF
