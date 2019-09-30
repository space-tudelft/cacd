
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

DATE = $(shell date)

OBJS =	classifyNet.o globals.o main.o makebus.o \
	seadifGraph.o showbus.o thedate.o

TARGET = makebus
TARGET2= showbus

install : $(TARGET)
	ln -s $(TARGET) $(TARGET2)
	mv $(TARGET) $(TARGET2) $(BINPATH)

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
