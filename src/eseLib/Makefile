
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

TARGET= libese.a

HDRS = eseOption.h

OBJS = eseOptHandler.o

install: $(TARGET)
	mv $(TARGET) $(LIBPATH)

$(TARGET): $(OBJS)
	ar crs $(TARGET) $(OBJS)
	ranlib $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

#EOF
