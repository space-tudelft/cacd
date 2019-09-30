
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

OBJS =	main.o

TARGET = addproj

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(OBJS) $(TARGET)

#END
