
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

TARGET= schur.a

HDRS = define.h export.h extern.h

OBJS = mem.o print.o schur.o

install: $(TARGET)
	mv $(TARGET) $(LIBPATH)

$(TARGET): $(OBJS)
	ar crs $(TARGET) $(OBJS)

clean:; $(RM) $(OBJS) $(TARGET)

#EOF
