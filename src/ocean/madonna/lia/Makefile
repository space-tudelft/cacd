
include ../../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../..

OBJS = array.o basearr.o box.o item.o list.o package.o

TARGET = liblia.a

install : $(TARGET)
	mv $(TARGET) $(LIBPATH)

%.o: %.C
	$(CCC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	ar crs $@ $^
	ranlib $@

clean:
	$(RM) $(OBJS) $(TARGET)

#EOF
