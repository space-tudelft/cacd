
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL = define.h extern.h type.h
LSRC = cmd_l.h.l
YSRC = cmd_y.y
OBJS = cmd_y.o main.o draw.o edit.o events.o read.o readSet.o res.o

TARGET = simeye2

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

cmd_l.h: $(LSRC)
	$(LEX)  -o $@ $(LSRC)
cmd_y.c: $(YSRC) cmd_l.h
	#$(YACC) -o $@ --defines=cmd_y.h $(YSRC)
	$(YACC) -o $@ $(YSRC)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ \
		-lXaw -lXm -lXt -lX11 \
		$(LIBPATH)/libddm.a -lm

clean:
	$(RM) $(TARGET) $(OBJS) cmd_l.h cmd_y.c cmd_y.h

#EOF
