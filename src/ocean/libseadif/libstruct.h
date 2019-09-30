/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Definitions of the main data types for the ocean programs.
 *
 * We need a work-around to avoid conflicts with the Yacc program y.tab.c.
 * The problem is that the symbols #define'd in seadif.y partly coincide
 * with the names of the typedefs in libstruct.h. For example, y.tab.c
 * defines:
 *
 *     #define SEADIF 261
 *
 * If the macro for getting a new SEADIF structure would be defined:
 *
 *     #define NewSeadif(p) {(p)=(SEADIFPTR)mnew(sizeof(SEADIF));}
 *
 * then we are clearly into trouble because mnew() is instructed to
 * arrange for an object of sizeof(int). The problem is fixed if the
 * name of the typedef is changed. That's why the following typedefs
 * have at least two names: SEADIF for convenience and SEADIF_TYPE to
 * avoid conflicts with Yacc.
 */

#ifndef __LIBSTRUC_H
#define __LIBSTRUC_H

#include "src/ocean/libseadif/systypes.h"   /* this defines time_t for STATUS.timestamp */

typedef union
{
  int   l;
  short s[2];
  void *p;
  char  v[4];
}
FLAG; /* extra user-available storage in a struct */

typedef char *STRING, *STRING_TYPE; /* Two strings are considered equal if they
					are identified by the same canonic STRING */
typedef struct
{
  time_t timestamp;	      /* Collapsed time since Unix birth date. */
  STRING author;	      /* Who created this stuff? */
  STRING program;	      /* ...or was it a program? */
}
STATUS, STATUS_TYPE, *STATUSPTR; /* General information structure. */

typedef struct
{
  STRING          filename;   /* Name of the Seadif file. */
  struct _LIBRARY *library;   /* List of libraries contained in the file. */
  STATUSPTR       status;     /* Miscellaneous lib-file information. */
}
SEADIF, SEADIF_TYPE, *SEADIFPTR; /* Top level Seadif structure. */

typedef struct _LIBRARY
{
  STRING           name;      /* Name of the library. */
  STRING           technology; /* Name of the technology. */
  STATUSPTR        status;    /* Miscellaneous library information. */
  struct _FUNCTION *function; /* List of available functions. */
  FLAG             flag;      /* support for algorithms */
  struct _LIBRARY  *next;     /* Pointer to next library in the file. */
}
LIBRARY, LIBRARY_TYPE, *LIBRARYPTR; /* Top level Seadif library. */

typedef struct _FUNCTION
{
  STRING           name;      /* Function name. */
  STRING           type;      /* Function type. */
  struct _CIRCUIT  *circuit;  /* List of function implementations. */
  STATUSPTR        status;    /* Miscellaneous function information. */
  LIBRARYPTR       library;   /* Library in which the function resides. */
  FLAG             flag;      /* support for algorithms */
  struct _FUNCTION *next;     /* Pointer to next function in the library. */
}
FUNCTION, FUNCTION_TYPE, *FUNCTIONPTR; /* Entry to functional equivalent circuits. */

typedef struct _CIRCUIT
{
  STRING          name;	      /* Name of the circuit. */
  struct _CIRPORT *cirport;   /* List of circuit's terminals. */
  struct _CIRINST *cirinst;   /* List of circuit's instances. */
  struct _NET     *netlist;   /* List of circuit's nets. */
  struct _BUS     *buslist;   /* Buses */
  struct _SHAPEF  *shapef;    /* The shape function for this circuit */
  struct _LAYOUT  *layout;    /* List of layed out circ. implementations. */
  struct _TIMING  *timing;    /* List timing models of the circuit */
  STATUSPTR       status;     /* Miscellaneous circuit information */
  STRING          attribute;  /* E.g. capacitance or transistor channel width */
  FUNCTIONPTR     function;   /* Backward reference to function. */
  FLAG            flag;	      /* user available storage */
  long            linkcnt;    /* Number of pointers to this circuit. */
  struct _CIRCUIT *next;      /* Pointer to next function implementation. */
}
CIRCUIT, CIRCUIT_TYPE, *CIRCUITPTR; /* Entry to circuit-equivalent layouts. */

typedef struct _SHAPEF
{
  short          bbx[2];      /* horiz and vert lenght in bbx[HOR] and bbx[VER] */
  struct _SHAPEF *next;	      /* pointer to next (h,v) pair */
}
SHAPEF, SHAPEF_TYPE, *SHAPEFPTR; /* Shape function, as a list of minimal (h,v) pairs.
				This is actually the list of bbx values in the layout
				list of the circuit, and therefore redundant. */
typedef struct _CIRINST
{
  STRING          name;	      /* Name of the instance. */
  CIRCUITPTR      circuit;    /* Actual reference to the instance. */
  CIRCUITPTR      curcirc;    /* Backward reference to current circuit. */
  STRING          attribute;  /* This overrules circuit->attribute */
  struct _CIRINST *next;      /* Next instance in the list. */
  FLAG            flag;	      /* support for algorithms */
}
CIRINST, CIRINST_TYPE, *CIRINSTPTR; /* List of circuit instances enables
				   multiple references to a single CIRCUIT. */

#ifdef SDF_PORT_DIRECTIONS
#define SDF_PORT_UNKNOWN 0
#define SDF_PORT_IN      1
#define SDF_PORT_OUT     2
#define SDF_PORT_INOUT   (SDF_PORT_IN | SDF_PORT_OUT)
#endif

typedef struct _CIRPORT
{
  STRING          name;	      /* Name of this terminal. */
  struct _NET     *net;	      /* Reference to principal (not parent) net. */
  struct _CIRPORT *next;      /* Next terminal of this circuit. */
  FLAG            flag;	      /* support for algorithms */
#ifdef SDF_PORT_DIRECTIONS
  int             direction;  /* one of SDF_PORT_{IN,OUT,INOUT} */
#endif
}
CIRPORT, CIRPORT_TYPE, *CIRPORTPTR; /* Circuit interface description. */

typedef struct _CIRPORTREF
{
  struct _NET        *net;    /* circuit's net that contains this cirport.*/
  CIRPORTPTR         cirport; /* Reference to actual terminal. */
  CIRINSTPTR         cirinst; /* This instance refers to corresponding circuit
			       * of cirport. Must be NULL for self reference. */
  struct _CIRPORTREF *next;   /* Next terminal in circuit's net. */
  FLAG               flag;    /* support for algorithms */
}
CIRPORTREF, CIRPORTREF_TYPE, *CIRPORTREFPTR; /* Basic component of the net list. */

typedef struct _NET
{
  STRING        name;	      /* Name of net. */
  int           num_term;     /* Number of CIRPORTREFs in this net. */
  CIRPORTREFPTR terminals;    /* List of joined terminals. */
  CIRCUITPTR    circuit;      /* Backward reference to the net's circuit. */
  struct _NET   *next;	      /* Next net in the net list. */
  FLAG          flag;	      /* support for algorithms */
}
NET, NET_TYPE, *NETPTR;	      /* List of all nets in a circuit. */

typedef struct _BUS
{
  STRING         name;	      /* Name of the bus */
  struct _NETREF *netref;     /* List of nets that make up this bus */
  struct _BUS    *next;	      /* Next bus */
  FLAG           flag;	      /* support for algorithms */
}
BUS, BUS_TYPE, *BUSPTR;	      /* List of wires that "belong together" */

typedef struct _NETREF
{
  struct _NET     *net;	      /* The net that (partially) belongs to the bus */
  CIRPORTREFPTR   *cirport;   /* Only this subset of the net belongs to the bus */
  struct _NETREF  *next;      /* Next reference to a net of the same bus */
  FLAG            flag;	      /* support for algorithms */
}
NETREF, NETREF_TYPE, *NETREFPTR; /* List of subsets of nets that form a bus */

#define HOR 0		      /* Legal indices for bbx[], off[] and pos[]. */
#define VER 1

typedef struct _LAYOUT
{
  STRING          name;	      /* Name of this layout. */
  struct _LAYPORT *layport;   /* List of layout's terminals. */
  struct _LAYLABEL *laylabel; /* List of layout's labels. */
  short           bbx[2];     /* Horizontal (bbx[HOR]) and vertical (bbx[VER])
			       * size of bounding box measured in grid points.*/
  short           off[2];     /* Horizontal (off[HOR]) and vertical (off[VER])
			       * offset with respect to basic image */
  struct _SLICE   *slice;     /* Floorplan and list of layout instances. */
  struct _WIRE    *wire;      /* List of wires for all layers. */
  STATUSPTR       status;     /* Miscellaneous information. */
  CIRCUITPTR      circuit;    /* Backward reference to circuit view. */
  long            linkcnt;    /* Number of pointers to this layout. */
  struct _LAYOUT  *next;      /* Next layed out circuit implementation. */
  FLAG            flag;	      /* support for algorithms */
}
LAYOUT, LAYOUT_TYPE, *LAYOUTPTR; /* Describes a circuit implementation. */

typedef struct _LAYPORT
{
  CIRPORTPTR      cirport;    /* Reference to the corresponding CIPORT. */
  short           layer;      /* Identifies the layer of this terminal. */
  short           pos[2];     /* Horizontal (pos[HOR]) and vertical (pos[VER])
			       * position with respect to LAYOUT.off. */
  struct _LAYPORT *next;      /* Reference to next layed out terminal. */
  FLAG            flag;	      /* support for algorithms */
}
LAYPORT, LAYPORT_TYPE, *LAYPORTPTR; /* Describes layout terminal implementation.*/

typedef struct _LAYLABEL
{
  STRING           name;      /* Name of the label. */
  short           layer;      /* Identifies the layer of this terminal. */
  short           pos[2];     /* Horizontal (pos[HOR]) and vertical (pos[VER])
                               * position with respect to LAYOUT.off. */
  struct _LAYLABEL *next;      /* Reference to next layed out terminal. */
  FLAG            flag;       /* support for algorithms */
}
LAYLABEL, LAYLABEL_TYPE, *LAYLABELPTR; /* Describes layout label implementation.*/

/* values for the mtx[] index */
#define A11 0
#define A12 1
#define A21 3
#define A22 4
#define B1  2
#define B2  5

typedef struct _LAYINST
{
  STRING          name;	      /* Instance name. */
  LAYOUTPTR       layout;     /* Reference to the actual layout. */
  short           mtx[6];     /* Orientation matrix. */
  struct _LAYINST *next;      /* Next element in the LAYINST list. */
  FLAG            flag;	      /* support for algorithms */
}
LAYINST, LAYINST_TYPE, *LAYINSTPTR; /* Enables multiple references to a layout. */

#define HORIZONTAL   0	      /* Legal values for SLICE.ordination */
#define VERTICAL     1
#define CHAOS        2

#define SLICE_CHLD   4	      /* Legal values for SLICE.chld */
#define LAYINST_CHLD 8

typedef struct _SLICE
{
  short             ordination;	/* HORIZONTAL, VERTICAL or CHAOS. */
  short             chld_type; /* SLICE_CHLD or LAYINST_CHLD. */
  union {
      struct _SLICE *slice;   /* Select this one if chld_type is SLICE. */
      LAYINSTPTR    layinst;  /* Select this one if chld_type is LAYINST. */
  }
  chld;			      /* Reference to list of children, which may *
			       * either be a SLICE list or a LAYINST list.*/
  struct _SLICE     *next;    /* Next element in the SLICE list. */
  FLAG              flag;     /* support for algorithms */
}
SLICE, SLICE_TYPE, *SLICEPTR; /* Enables slicing floorplan specification. */

#define XL 0		      /* Legal indices for WIRE.crd[]. */
#define XR 1
#define YB 2
#define YT 3

typedef struct _WIRE
{
  short        crd[4];	      /* x-left,x-right,y-bottom and y-top coordinates. */
  short        layer;	      /* z-coordinate. */
  struct _WIRE *next;	      /* Next wire of layout. */
}
WIRE, WIRE_TYPE, *WIREPTR;    /* Nice rectangle for decribing a layout. */

/* Following macros allocate memory for the datastructure, see memman(3SDF) */
#define NewStatus(p)	((p)=(STATUSPTR)mnew(sizeof(STATUS_TYPE)))
#define NewSeadif(p)	((p)=(SEADIFPTR)mnew(sizeof(SEADIF_TYPE)))
#define NewLibrary(p)	((p)=(LIBRARYPTR)mnew(sizeof(LIBRARY_TYPE)))
#define NewProptab(p)	((p)=(PROPTABPTR)mnew(sizeof(PROPTAB_TYPE)))
#define NewFunction(p)	((p)=(FUNCTIONPTR)mnew(sizeof(FUNCTION_TYPE)))
#define NewCircuit(p)	((p)=(CIRCUITPTR)mnew(sizeof(CIRCUIT_TYPE)))
#define NewShapef(p)	((p)=(SHAPEFPTR)mnew(sizeof(SHAPEF_TYPE)))
#define NewCirinst(p)	((p)=(CIRINSTPTR)mnew(sizeof(CIRINST_TYPE)))
#define NewCirport(p)	((p)=(CIRPORTPTR)mnew(sizeof(CIRPORT_TYPE)))
#define NewCirportref(p) ((p)=(CIRPORTREFPTR)mnew(sizeof(CIRPORTREF_TYPE)))
#define NewNet(p)	((p)=(NETPTR)mnew(sizeof(NET_TYPE)))
#define NewNetRef(p)	((p)=(NETREFPTR)mnew(sizeof(NETREF_TYPE)))
#define NewBus(p)	((p)=(BUSPTR)mnew(sizeof(BUS_TYPE)))
#define NewLayout(p)	((p)=(LAYOUTPTR)mnew(sizeof(LAYOUT_TYPE)))
#define NewLayport(p)	((p)=(LAYPORTPTR)mnew(sizeof(LAYPORT_TYPE)))
#define NewLaylabel(p)	((p)=(LAYLABELPTR)mnew(sizeof(LAYLABEL_TYPE)))
#define NewLayinst(p)	((p)=(LAYINSTPTR)mnew(sizeof(LAYINST_TYPE)))
#define NewSlice(p)	((p)=(SLICEPTR)mnew(sizeof(SLICE_TYPE)))
#define NewWire(p)	((p)=(WIREPTR)mnew(sizeof(WIRE_TYPE)))

/* Following macros release memory from the datastructure, see memman(3SDF) */
#define FreeStatus(p) \
      { if ((p)->author) fs((p)->author); \
        if ((p)->program) fs((p)->program); \
        mfree((char **)(p), sizeof(STATUS_TYPE));}
#define FreeSeadif(p) \
      { if ((p)->filename) fs((p)->filename); \
        mfree((char **)(p), sizeof(SEADIF_TYPE)); }
#define FreeLibrary(p) \
      { if ((p)->name) fs((p)->name); \
	if ((p)->technology) fs((p)->technology); \
        mfree((char **)(p), sizeof(LIBRARY_TYPE)); }
#define FreeProptab(p) mfree((char **)(p), sizeof(PROPTAB_TYPE))
#define FreeFunction(p) \
      { if ((p)->name) fs((p)->name); \
        if ((p)->type) fs((p)->type); \
	mfree((char **)(p), sizeof(FUNCTION_TYPE)); }
#define FreeCircuit(p) \
      { if ((p)->name) fs((p)->name); \
        if ((p)->attribute) fs((p)->attribute); \
	mfree((char **)(p), sizeof(CIRCUIT_TYPE)); }
#define FreeShapef(p) mfree((char **)(p), sizeof(SHAPEF_TYPE))
#define FreeCirinst(p) \
      { if ((p)->name) fs((p)->name); \
        if ((p)->attribute) fs((p)->attribute); \
	mfree((char **)(p), sizeof(CIRINST_TYPE)); }
#define FreeCirport(p) \
      { if ((p)->name) fs((p)->name); \
	mfree((char **)(p), sizeof(CIRPORT_TYPE)); }
#define FreeCirportref(p) mfree((char **)(p), sizeof(CIRPORTREF_TYPE))
#define FreeNet(p) \
      { if ((p)->name) fs((p)->name); \
	mfree((char **)(p), sizeof(NET_TYPE)); }
#define FreeNetRef(p)	mfree((char **)(p), sizeof(NETREF_TYPE))
#define FreeBus(p) \
      { if ((p)->name) fs((p)->name); \
	mfree((char **)(p), sizeof(BUS_TYPE)); }
#define FreeLayout(p) \
      { if ((p)->name) fs((p)->name); \
	mfree((char **)(p), sizeof(LAYOUT_TYPE)); }
#define FreeLayport(p)	mfree((char **)(p), sizeof(LAYPORT_TYPE))
#define FreeLaylabel(p) \
      { if ((p)->name) fs((p)->name); \
        mfree((char **)(p), sizeof(LAYLABEL_TYPE)); }
#define FreeLayinst(p) \
      { if ((p)->name) fs((p)->name); \
	mfree((char **)(p), sizeof(LAYINST_TYPE)); }
#define FreeSlice(p)	mfree((char **)(p), sizeof(SLICE_TYPE))
#define FreeWire(p)	mfree((char **)(p), sizeof(WIRE_TYPE))

#if defined(__cplusplus)
#define __CPLUSPLUS__
#endif
				  /* IK, timing model structures  */
#include "src/ocean/libseadif/tm_struct.h"

/* useful stuff */
#undef  NULL
#define NULL 0

#define TRUE 1

#include <limits.h>
#ifndef MAXINT
#define MAXINT INT_MAX
#endif
#ifndef MININT
#define MININT (-MAXINT)
#endif

#define PRIVATE static

/* You can specify whether you want the sealib library to have C++ linkage or
 * normal C linkage by setting the LINKAGE_TYPE. Actually, there is no reason
 * to specify C++...  Setting this to C++ prevends you from linking the sealib
 * with normal C-compiled programs. Setting this to C leaves you free to link
 * the sealib with both C and C++ programs.
 */
#define LINKAGE_TYPE "C"

/* these used to be functions, until the day I profiled some stuff ... */
#define canonicstring(s) cs(s)
#define forgetstring(s)  fs(s)

#endif
