
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL =	define.h extern.h type.h
LSRC =	cmd_l.l
YSRC =	cmd_y.y
OBJS1 = cmd_y.o cmd_l.o main_pp.o readCom.o readCir.o res.o
OBJS2 = main_bs.o readCir.o

TARGET1 = nspice_pp
TARGET2 = nspice_bs

install : $(TARGET1) $(TARGET2)
	mv $(TARGET1) $(TARGET2) $(BINPATH)
	cp dospice $(BINPATH); chmod 755 $(BINPATH)/dospice
	cp nspice  $(BINPATH); chmod 755 $(BINPATH)/nspice

cmd_y.c: $(YSRC)
	$(YACC) -o $@ --defines=cmd_y.h $(YSRC)
cmd_l.c: $(LSRC) cmd_y.h
	$(LEX)  -o $@ $(LSRC)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET1): $(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a
$(TARGET2): $(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET1) $(TARGET2) $(OBJS1) $(OBJS2)
	$(RM) cmd_l.c cmd_y.c cmd_y.h

#EOF
