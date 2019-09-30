
BINPATH=$(ICDPATH)/bin
LIBPATH=$(ICDPATH)/lib

CC  = /usr/bin/gcc
CCC = /usr/bin/g++
LEX = /usr/bin/flex
YACC= /usr/bin/bison

# -D_FILE_OFFSET_BITS=64
OCFLAGS=-fPIC -O
LDFLAGS=-fPIC -s -L/usr/lib64

QTLIB=/usr/lib64/qt-3.3

