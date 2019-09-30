
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

TARGET= libX2ps.a

HDRS = X2PostScript.h X2PostScriptP.h X2psapi.h X2pslib.h

OBJS = X2pslib.o X2psapi.o X2PostScript.o

install: $(TARGET)
	mv $(TARGET) $(LIBPATH)

$(TARGET): $(OBJS)
	ar crs $(TARGET) $(OBJS)
	ranlib $(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET)

#EOF
