
include ../../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../..

DATE = $(shell date)

OBJS = main.o thedate.o

TARGET = madonna

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.C
	$(CCC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c $<

$(TARGET): $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $^ \
		$(LIBPATH)/libpartitioner.a \
		$(LIBPATH)/libphil.a \
		$(LIBPATH)/liblia.a \
		$(LIBPATH)/libocean.a \
		$(LIBPATH)/libseadif.a
	$(RM) thedate.o

clean:
	$(RM) $(OBJS) $(TARGET)

#EOF
