.T= "Files used by the Makegln Tools"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE APPLICATION NOTE
ABOUT
MAKEGLN TOOLS
SOURCE FILES
.S
.sp 1
.I
S. de Graaf
.sp 1
.R
Circuits and Systems Group
Faculty of Electrical Engineering
Delft University of Technology
The Netherlands
.DE
.sp 2c
.ce
Report ET-CAS 00-02
.ce
March 13, 2000
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2000-2004 by the author.

Last revision: December 4, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This report describes the files used for making one of the
.P= makegln
tools.
The tools are used for preprocessing the gln-data for the
.P= space
extractor.

It are the files of the following four tools:
.BL
.LI
makegln
.LI
makedela
.LI
makemesh
.LI
makesize
.LE

This work is carried out for the \*(sP project.
.SP 12
.nr Ej 0
.H 1 "REFERENCES"
.nf
.ta .7c
[1]	N.P. van der Meijs, A.J. van Genderen, "Space User's Manual",
	report ET-NT 92.21, 1999.

[2]	N.P. van der Meijs, A.J. van Genderen, "Space Tutorial",
	report ET-NT 92.22, 1999.

.nr Ej 1
.HU "APPENDIX A -- Compilation flags for HP700"
.fS
-----------------------------------------------------------------
CFLAGS= -O
        -I/usr/include/X11R5 -I/usr/include/Motif1.2
        -I/usr/include/X11R5mit -I/usr/local/include
        -W p,-H1000000 +DA1.0 +DS1.0 -DLICENSE -O -DANNOTATE
        -I. -I/u/25/25/space2/Shadow/ddtc/src/eseLib
        -DSYSV
        -I/u/25/25/space2/Shadow/ddtc/lib/include
        -I..

LDFLAGS=-Wl,-a,archive
        -L/usr/lib/X11R5 -L/usr/lib/Motif1.2
        -L/usr/lib/X11R5mit -L/usr/local/lib -L/usr/local/lib/X11R5
        $(CFLAGS)
        $(OBJECTS)
        ../auxil/auxil.a
        /u/25/25/space2/Shadow/ddtc/lib/liblayfmt.a
        /u/25/25/space2/Shadow/ddtc/lib/libddm.a
        -ll -lm -lbsdipc -lBSD

MFLAGS= -bn
        OCFLAGS="-I/usr/include/X11R5 -I/usr/include/Motif1.2
                -I/usr/include/X11R5mit -I/usr/local/include
                -W p,-H1000000 +DA1.0 +DS1.0 -DLICENSE -O -DANNOTATE
                -I. -I/u/25/25/space2/Shadow/ddtc/src/eseLib"
        CC="cc"
        CACDSRCHOME=/u/25/25/space2/Shadow/ddtc
        ARCHITECTURE=ddtc
        ICDPATH=/u/25/25/space2/Shadow/ddtc
        OSTYPE=SYSV
        LDFLAGS="-Wl,-a,archive -L/usr/lib/X11R5 -L/usr/lib/Motif1.2
                -L/usr/lib/X11R5mit -L/usr/local/lib -L/usr/local/lib/X11R5"
        CACDCONFIG=/u/25/25/space2/Shadow/ddtc/CONFIG/M.ddtc
        OLIBS="-ll -lm -lbsdipc -lBSD"

-----------------------------------------------------------------
% make -n makegln

D=src/space/auxil; test -d $D || exit 0; cd $D; echo ===== $D ====; \\
make $(MFLAGS) install
===== src/space/auxil ====

D=src/space/makegln; test -d $D || exit 0; cd $D; echo ===== $D ====; \\
make $(MFLAGS) LIBESE="/u/25/25/space2/Shadow/ddtc/src/eseLib/libese.a" install
===== src/space/makegln ====
.fE
.HU "APPENDIX B -- Files for the tools"
.fS
-----------------------------------------------------------------
make -n makegln
-----------------------------------------------------------------
    SRC/regress.sh (Shell procedures for regression testing)
    SRC/config.h
    SRC/makegln.h
    SRC/proto.h

    cc $(CFLAGS) -DPACK -c input.c
    cc $(CFLAGS) -DPACK -c edge.c
    cc $(CFLAGS) -DPACK -c main.c
    cc $(CFLAGS) -DPACK -c scan.c
    cc $(CFLAGS) -DPACK -c output.c
    cc $(CFLAGS) -DPACK -c slant.c
    cc $(CFLAGS) -DPACK -c gln.c
    cc $(CFLAGS) -DPACK -c sort.c
    cc $(CFLAGS) -DPACK -c qsort.c
    cc $(CFLAGS) -DPACK -c pqueue.c
    cc $(LDFLAGS) -o makegln

 *) ../makegln/*.c (SRC)
-----------------------------------------------------------------
make -n makesize
-----------------------------------------------------------------
    SRC: extern.h, makesize.h, mkglnexp.h, rscan.h
    ../makegln/config.h
    ../makegln/makegln.h
    ../makegln/proto.h

 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c redge.c
 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c rscan.c
 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c tile.c
 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c stubs.c
 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c hier.c
 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c rslant.c
 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c input.c
 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c update.c
 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c makesize.c
 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c main.c
 1) cc -DMAKESIZE $(CFLAGS) -DPACK -c extract.c
 2) cc -DMAKESIZE $(CFLAGS) -DPACK -c gettech.c
 2) cc -DMAKESIZE $(CFLAGS) -DPACK -c recog.c
    cc -DMAKESIZE $(CFLAGS) -DPACK -c sort.c
    cc -DMAKESIZE $(CFLAGS) -DPACK -c scan.c
    cc -DMAKESIZE $(CFLAGS) -DPACK -c gln.c
    cc -DMAKESIZE $(CFLAGS) -DPACK -c edge.c
    cc -DMAKESIZE $(CFLAGS) -DPACK -c output.c
    cc -DMAKESIZE $(CFLAGS) -DPACK -c pqueue.c
    cc -DMAKESIZE $(CFLAGS) -DPACK -c qsort.c
    cc -DMAKESIZE $(CFLAGS) -DPACK -c slant.c
    cc -DMAKESIZE $(LDFLAGS) -o makesize

 1) ../makesize/*.c (SRC)
 2) ../extract/*.c
 *) ../makegln/*.c
-----------------------------------------------------------------
.fE
.fS
-----------------------------------------------------------------
make -n makedela
-----------------------------------------------------------------
    SRC/dproto.h
    SRC/makedela.h
    ../makegln/config.h
    ../makegln/makegln.h
    ../makegln/proto.h

    cc -DMAKEDELA $(CFLAGS) -c main.c
    cc -DMAKEDELA $(CFLAGS) -c edge.c
    cc -DMAKEDELA $(CFLAGS) -c slant.c
    cc -DMAKEDELA $(CFLAGS) -c sort.c
    cc -DMAKEDELA $(CFLAGS) -c qsort.c
    cc -DMAKEDELA $(CFLAGS) -c pqueue.c
 2) cc -DMAKEDELA $(CFLAGS) -c input.c
 1) cc -DMAKEDELA $(CFLAGS) -c scan.c
 1) cc -DMAKEDELA $(CFLAGS) -c output.c
    cc -DMAKEDELA $(LDFLAGS) -o makedela

 1) ../makedela/*.c (SRC)
 2) ../makemesh/*.c
 *) ../makegln/*.c
-----------------------------------------------------------------
make -n makemesh
-----------------------------------------------------------------
    ../makegln/config.h
    ../makegln/makegln.h
    ../makegln/proto.h

 1) cc -DMAKEMESH $(CFLAGS) -c input.c
    cc -DMAKEMESH $(CFLAGS) -c edge.c
    cc -DMAKEMESH $(CFLAGS) -c main.c
    cc -DMAKEMESH $(CFLAGS) -c scan.c
    cc -DMAKEMESH $(CFLAGS) -c output.c
    cc -DMAKEMESH $(CFLAGS) -c slant.c
 1) cc -DMAKEMESH $(CFLAGS) -c gln.c
    cc -DMAKEMESH $(CFLAGS) -c sort.c
    cc -DMAKEMESH $(CFLAGS) -c qsort.c
    cc -DMAKEMESH $(CFLAGS) -c pqueue.c
    cc -DMAKEMESH $(LDFLAGS) -o makemesh

 1) ../makemesh/*.c (SRC)
 *) ../makegln/*.c
-----------------------------------------------------------------
.fE
Note that
.P= makegln
and
.P= makesize
use the PACK flag during compilation, but this is not more needed.
There is no local read/write function which is using this define.
.HU "APPENDIX C -- Control flow of the tools"
.SP 2
.fS
           info3 -> +----------+
         LC_bxx --> | makegln  | --> LC_gln
       t_LC_bxx     +----------+


           info3 -> +----------+
   1)  cont_aln --> | makedela | --> delaunay (for "substr/subcont.c")
                    +----------+ --> delgraph (for "X11/draw.c")
                                 --> delaunay.pic

           info3 -> +----------+
   2)  mesh_aln --> | makemesh | --> mesh_gln
                    +----------+


           info3 -> +----------+ -> info3
   3)    LC_gln --> | makesize | --> resize.t
                    +----------+ --> makesize_gln --> LC_gln


   1) Written by "extract/contedge.c".
   2) Written by "extract/meshedge.c".
   3) Written by "makegln".
.fE
.SP 4
Note that all the tools can use a temp. disk file for storage during sorting
of the gln-edges.  This disk file is possibly compressed.
.TC 2 1 3 0
