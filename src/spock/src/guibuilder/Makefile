
include ../../../Config.mk

CWD = $(shell pwd)

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../.. -I$(QTLIB)/include -DQT_NO_DEBUG=1

OBJM =	DataConnection_moc.cc GeneratorVisitor_moc.cc GUIBuilderVisitor_moc.cc \
	GUITree_moc.cc Process_moc.cc SerializerVisitor_moc.cc

OBJS =	DataConnection.o DataConnection_moc.o GeneratorVisitor_moc.o \
	GUIBuilderVisitor.o GUIBuilderVisitor_moc.o GUITree.o GUITree_moc.o \
	Process.o Process_moc.o SerializerVisitor.o SerializerVisitor_moc.o

TARGET = libGUIBuilder.a

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@

%.o: %.cc
	$(CCC) $(CFLAGS) -c $<

DataConnection_moc.cc: DataConnection.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
GeneratorVisitor_moc.cc: GeneratorVisitor.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
GUIBuilderVisitor_moc.cc: GUIBuilderVisitor.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
GUITree_moc.cc: GUITree.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
Process_moc.cc: Process.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
SerializerVisitor_moc.cc: SerializerVisitor.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@

clean:
	$(RM) $(TARGET) $(OBJS) $(OBJM)

#EOF
