
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL = extern.h gdef.h incl.h proto.h
LSRC = ldm_lex.l
LSRC2= cif_lex.l
YSRC = ldm_parse.y
YSRC2= cif_parse.y
OBJS =	files.o func.o ini_mtree.o man_tree.o pr_exit.o proc_box.o \
	proc_circ.o proc_cont.o proc_mc.o proc_poly.o proc_sbox.o \
	proc_term.o proc_label.o proc_wire.o proc_swire.o write_info.o
OBJS1=	main.o ldm_parse.o ldm_lex.o
OBJS2=	cif_main.o cif_parse.o cif_lex.o

TARGET1= cldm
TARGET2= ccif

install : $(TARGET1) $(TARGET2)
	mv $(TARGET1) $(TARGET2) $(BINPATH)

ldm_lex.c: $(LSRC) ldm_parse.h
	$(LEX) -o ldm_lex.c $(LSRC)
ldm_parse.c: $(YSRC)
	$(YACC) -o ldm_parse.c --defines=ldm_parse.h $(YSRC)
cif_lex.c: $(LSRC2) cif_parse.h
	$(LEX) -o cif_lex.c $(LSRC2)
cif_parse.c: $(YSRC2)
	$(YACC) -o cif_parse.c --defines=cif_parse.h $(YSRC2)
cif_main.c:
	ln -s main.c $@

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<
main.o: main.c $(INCL)
	$(CC) $(CFLAGS) -DCLDM=1 -c $<

$(TARGET1): $(OBJS) $(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm
$(TARGET2): $(OBJS) $(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm

clean:
	$(RM) $(TARGET1) $(TARGET2) $(OBJS) $(OBJS1) $(OBJS2) cif_main.c
	$(RM) cif_lex.c cif_parse.c cif_parse.h
	$(RM) ldm_lex.c ldm_parse.c ldm_parse.h

#EOF
