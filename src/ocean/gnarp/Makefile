
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

DATE = $(shell date)

OBJS = addCirports.o doGnarp.o printObjects.o status.o main.o thedate.o

TARGET = gnarp

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.C
	$(CCC) $(CFLAGS) -c $<
thedate.o: thedate.C
	$(CCC) $(CFLAGS) -DCOMPILE_DATE_STRING="\"$(DATE)\"" -c $<

$(TARGET): $(OBJS)
	$(CCC) $(LDFLAGS) -o $@ $^ \
		$(LIBPATH)/libocean.a \
		$(LIBPATH)/libseadif.a
	$(RM) thedate.o

clean:
	$(RM) $(OBJS) $(TARGET)

#EOF
