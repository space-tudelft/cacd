
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL = define.h extern.h type.h
ASRC = simeye.res
LSRC = cmd_l.h.l
YSRC = cmd_y.y
OBJS = cmd_y.o main.o draw.o edit.o events.o read.o readSet.o res.o
OBJ2 = arrexp.o

TARGET = simeye
TARGET2 = arrexp
APPDIR = $(ICDPATH)/share/lib/app-defaults

install: $(TARGET) $(TARGET2)
	mv $(TARGET) $(TARGET2) $(BINPATH)
	if [ ! -d $(APPDIR) ]; then mkdir $(APPDIR); fi
	rm -f $(APPDIR)/Simeye
	cp $(ASRC) $(APPDIR)/Simeye

cmd_l.h: $(LSRC)
	$(LEX)  -o $@ $(LSRC)
cmd_y.c: $(YSRC) cmd_l.h
	#$(YACC) -o $@ --defines=cmd_y.h $(YSRC)
	$(YACC) -o $@ $(YSRC)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ \
		$(LIBPATH)/libX2ps.a -lXm -lXt -lX11 \
		$(LIBPATH)/libddm.a -lm

$(TARGET2): $(OBJ2)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(TARGET) $(TARGET2) $(OBJS) $(OBJ2)
	$(RM) cmd_l.h cmd_y.c cmd_y.h

#EOF
