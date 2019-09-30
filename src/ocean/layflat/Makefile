
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

DATE = $(shell date)

OBJS = bbx_stuff.o err.o flattenmc.o main.o mtx.o put_stuff.o term_stuff.o thedate.o

TARGET = layflat

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
thedate.o: thedate.c
	$(CC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c $<

$(TARGET): $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $^ \
		$(LIBPATH)/libnelsea.a \
		$(LIBPATH)/libddm.a \
		$(LIBPATH)/libocean.a \
		$(LIBPATH)/libseadif.a
	$(RM) thedate.o

clean:
	$(RM) $(OBJS) $(TARGET)

#EOF
