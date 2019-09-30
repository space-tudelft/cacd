
include ../../../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../../.. -I$(QTLIB)/include -DQT_NO_DEBUG=1

OBJS = Helper.o Log.o

TARGET = liblog.a

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@

%.o: %.cc
	$(CCC) $(CFLAGS) -c $<

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
