
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

OBJ1 = subresgen.o
OBJ2 = subresmkdir.o

TARGET1 = subresgen
TARGET2 = subresmkdir

install: $(TARGET1) $(TARGET2)
	mv $(TARGET1) $(TARGET2) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET1): $(OBJ1)
	$(CC) $(LDFLAGS) -o $@ $^
$(TARGET2): $(OBJ2)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(TARGET1) $(OBJ1) $(TARGET2) $(OBJ2)

#EOF
