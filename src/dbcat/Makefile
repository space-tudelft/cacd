
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

OBJS1=	main.o
OBJS2=	main2.o

TARGET1= dbcat
TARGET2= dbcir

install : $(TARGET1) $(TARGET2)
	mv $(TARGET1) $(TARGET2) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET1): $(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a
$(TARGET2): $(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET1) $(TARGET2) $(OBJS1) $(OBJS2)

#EOF
