
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

OBJS =	cacdcmap.o cmaperror.o
OBJ1 =	main.o cmaps.o setcmap.o

TARGET = libcmap.a
TARGET1= setcmap

install : $(TARGET) $(TARGET1)
	mv $(TARGET1) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@
	mv $@ $(LIBPATH)

$(TARGET1): $(OBJ1)
	$(CC) $(LDFLAGS) -o $@ $(OBJ1) \
			$(LIBPATH)/libcmap.a -lX11

clean:
	$(RM) $(OBJS) $(OBJ1) $(TARGET) $(TARGET1)

#EOF
