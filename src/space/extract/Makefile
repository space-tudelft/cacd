
include ../../Config.mk

CFLAGS=-DDRIVER -Wall -Wno-unused $(OCFLAGS) -I../../..

TARGET= gettech

SRCS = gettech.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ \
		$(LIBPATH)/auxil.a \
		$(LIBPATH)/libddm.a

clean:; $(RM) $(TARGET)

#EOF
