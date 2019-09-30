
include ../../../Config.mk

CFLAGS =-Wall -Wno-unused $(OCFLAGS) -I../../../..

TARGET1= minimize
TARGET2= mintest

OBJ1 =	minimize.o
OBJ2 =	mintest.o

%.o: %.cc
	$(CCC) $(CFLAGS) -c $<

makeall: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJ1)
	$(CCC) $(LDFLAGS) $(OBJ1) -o $(TARGET1) \
		$(LIBPATH)/libmin.a

$(TARGET2): $(OBJ2)
	$(CCC) $(LDFLAGS) $(OBJ2) -o $(TARGET2) \
		$(LIBPATH)/libmin.a

install: $(TARGET1)
	cp $(TARGET1) $(BINPATH)/$(TARGET1)
	chmod 755 $(BINPATH)/$(TARGET1)

clean:; $(RM) $(OBJ1) $(TARGET1) $(OBJ2) $(TARGET2)

#EOF
