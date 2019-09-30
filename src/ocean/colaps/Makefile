
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

DATE = $(shell date)

OBJS =	cleanUp.o colapsInst.o colapsNet.o globalVar.o liftAll.o localCopy.o \
	makeName.o printPath.o readNonFlat.o main.o thedate.o

TARGET = colaps

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
