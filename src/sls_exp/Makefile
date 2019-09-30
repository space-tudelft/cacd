
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL = define.h extern.h gndefine.h gntype.h search.h type.h

OBJS = compact.o dberror.o deb.o find.o findhash.o getdev.o getnet.o getterm.o \
	hiername.o inbin.o linkntw.o main.o modtype.o mksim.o new.o outbin.o \
	rmnoterm.o sort.o vfunc.o bsearch.o hsearch.o monit.o

TARGET = sls_exp

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
