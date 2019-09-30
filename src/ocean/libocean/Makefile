
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS =	format.o Array.o BinaryTree.o Char.o Container.o Double.o Int.o \
	Iterator.o Long.o Nil.o Object.o Point.o Rectangle.o RedBlackTree.o \
	Root.o Sortable.o String.o graph.o graphSteiner.o hashTable.o \
	priorQ.o sdfGraph.o sdfNameIter.o sdfset.o regex.o

TARGET = libocean.a

install : $(TARGET)
	mv $(TARGET) $(LIBPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
%.o: %.C
	$(CCC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	ar crs $@ $^
	ranlib $@

clean:
	$(RM) $(OBJS) $(TARGET)

#EOF
