/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#ifndef __DMINCL_H
#define __DMINCL_H

#include <stdio.h>

/*
** This will define the current release of the Nelsis CAD Framework
*/
#ifndef NCF_RELEASE
#define NCF_RELEASE     300
#endif /* NCF_RELEASE */

/*
** ICD defines
*/
#define DM_MAXNOMASKS	2048
#define DM_MAXNAME	255   /* release 302 */
#define DM_MAXNAME_OLD1	32    /* release 301 */
#define DM_MAXNAME_OLD  14    /* release 3   */
#define DM_MAXLAY	32
#define DM_MAXLINE    1024

#define DM_MAXPROCESSES	128
#define DM_MAXPROJECTS	128
#define DM_MAXCELLS	20480
#define DM_MAXSTREAMS	800
#define DM_MAXPATHLEN	1024

/*
** DM_NOVIEWS == sizeof (dmviews) / sizeof (char *)
*/
#define	DM_NOVIEWS	3

/*
** Xcontrol defines
*/
#define DM_CT_REGULAR	1
#define DM_CT_MACRO	2
#define DM_CT_DEVICE	3
#define DM_CT_LIBRARY	4
#define DM_CT_IMPORT	5

#define DM_IF_STRICT	1
#define DM_IF_FREE	2
#define DM_IF_FREEMASKS	3

typedef struct {
    char   *name;		/* cell name */
    long    timestamp;		/* last modified date */
    short   celltype;		/* type of cell */
    short   interfacetype;	/* type of interface */
    char   *masks;		/* list of masks */
} DM_XDATA;

/*
** Process defines
*/
#define	DM_OTHER_MASK	0
#define	DM_INTCON_MASK	1
#define	DM_SYMB_MASK	2

typedef struct {
    int     nomasks;		/* number of masks */
    char   *pr_name;		/* process name */
    char   *pr_type;		/* process type */
    char   *pr_info;		/* process info */
    char  **mask_name;		/* mask name */
    char  **mask_info;		/* mask name info */
    int    *mask_no;		/* mask number */
    int    *mask_type;		/* mask type */
    int    *pgt_no;		/* if(1) process this mask */
    int    *pgt_type;		/* pgt pos./neg. mask number */
    struct device {
	int     code;		/* color code */
	int     fill;		/* filling info */
    } **CM, **RT, **PLOT;
} DM_PROCDATA;

/*
** Error defines
*/
extern char *dmerrlist[];
extern int   dmnerr;
extern int   dmerrno;

#define DME_SYS 	1	/* error in system call */
#define DME_DDM		2	/* DDM internal error */
#define DME_BADARG  	3	/* bad arguments call */
#define DME_MODE	4	/* mode is invalid for key */
#define DME_BADPR	5	/* bad project */
#define DME_BADVIEW	6	/* bad view */
#define DME_NOCELL	7	/* cell does not exist */
#define DME_BADKEY      8	/* bad key */
#define DME_LOCK        9	/* cell is locked */
#define DME_PRIM        10	/* can't open primary file */
#define DME_RECUR       11	/* illegal recursion */
#define DME_EXIST	12	/* cell already exists */
#define DME_TECH	13	/* can't open technology file */
#define DME_PUT		14	/* dmPutDesignData write error */
#define DME_GET		15	/* dmGetDesignData read error */
#define DME_FMT		16	/* bad format */
#define DME_PRID	17	/* unknown process id */
#define DME_PRDATA	18	/* can't access process data */
#define DME_CORE	19	/* no more core */
#define DME_PRREAD	20	/* process data read error */
#define DME_CVIEW	21	/* bad current view */
#define DME_BADNAME	22	/* bad name */
#define DME_NOVIEW	23	/* view does not exist */
#define DME_EXVIEW	24	/* view already exists */
#define DME_PRLOCK	25	/* project is locked */
#define DME_FOPEN 	26	/* cannot open file */
#define DME_BADENV	27	/* bad DDM environment */
#define DME_NOCELLL	28	/* no celllist present */
#define DME_INIT	29	/* already initialized */
#define DME_NOPRJL	30	/* no project list present */
#define DME_NOIMPCL	31	/* no imported celllist present */
#define DME_NODMRC 	32	/* cannot open file .dmrc */
#define DME_BADREL	33	/* bad release */
#define DME_NOINIT	34	/* not yet initialized */
#define DME_DMANERR	35	/* design manager error */
#define DME_DMANRR	36	/* design manager rejects request */
#define DME_NOROOT	37	/* cell is not a root */
#define DME_NOFATH	38	/* no father cell */
#define DME_NOCELLTYPE	39	/* no/wrong celltype */
#define DME_NOINTERFACETYPE	40	/* no/wrong interfacetype */
#define DME_NOMASK	41	/* no/wrong mask present */

#define PROJ_READ	01
#define PROJ_WRITE	02
#define DEFAULT_MODE	(PROJ_READ | PROJ_WRITE)
#define DEFAULT_PROJECT	(char*)"."

#define LOCAL		0
#define IMPORTED	1

/*
** DDM version types
*/
#define		ACTUAL		"actual"
#define         WORKING         "working"
#define		BACKUP		"backup"
#define		DERIVED		"derived"
#define		DONTCARE	0

/*
** DDM views
*/
#define		LAYOUT		"layout"
#define		CIRCUIT		"circuit"
#define		FLOORPLAN	"floorplan"

/*
** modes for dmCheckOut
*/
#define         READONLY       	01
#define         UPDATE          02
#define         ATTACH          04
#define         CREATE          010
#define         INT_CREATE      020   /* used internally only */

/*
** modes for dmCheckIn
*/
#define		CONTINUE	0
#define		COMPLETE	1
#define		QUIT		2

/*
** modes for dmGetMetaDesignData
*/
#define IMPORTEDCELLLIST	0
#define CELLEQUIVALENCE		1
#define PROCESS			2
#define CELLLIST		3
#define PROJECTLIST		4
#define PROCPATH		7
#define CELLISROOT		8
#define FATHERCELL		9

/*
** non-orthogonal element codes
*/
#define	 	RECT_NOR	1
#define	 	POLY_NOR	2
#define	 	CIRCLE_NOR	3
#define	 	SBOX_NOR	4
#define	 	WIRE_NOR	5

/*
** the following are for the annotate stream
*/
#define GA_FORMAT       0
#define GA_LINE         1
#define GA_TEXT         2
#define GA_LABEL        3
#define GA_NO_ARROW     0
#define GA_FW_ARROW     1
#define GA_BW_ARROW     2
#define GA_DB_ARROW     3
#define GA_LEFT         0
#define GA_RIGHT        1
#define GA_CENTER       2

/*
** the following defines are the internal format identifications
** for the dmGetDesignData and dmPutDesignData functions
*/
#define GEO_INFO	0
#define GEO_BOX		1
#define GEO_MC		2
#define GEO_LUP		3
#define GEO_TERM	4
#define GEO_NOR_INI	5
#define GEO_NOR_XY	6
#define GEO_BOXLAY	7
#define GEO_TERMLAY	8
#define GEO_VLNLAY	9
#define GEO_TEQ		10
#define GEO_TID		11
#define GEO_INFO2	12
#define GEO_INFO3	13
#define	GEO_NXX_INI	14
#define GEO_NXX_XY	15
#define GEO_ABSTR	16
#define GEO_EXPSET	17
#define GEO_PLOT	18
#define GEO_SPEC	19
#define GEO_FLOC        20
#define GEO_GLN         21
#define GEO_ANNOTATE    22

#define CIR_MC		100
#define CIR_TERM	101
#define CIR_NET		102
#define CIR_SLS		103
#define CIR_INFO	104
#define CIR_GRAPHIC	105
#define CIR_SWIFT	106
#define CIR_FAULT	107
#define CIR_NET_ATOM    108
#define CIR_NET_HEAD    109

#define FLP_MC          200
#define FLP_INFO        201
#define FLP_TERM        202
#define FLP_CHAN        203

/*
** structure templates for data transfer with the
** dmGetDesignData and dmPutDesignData functions.
*/
struct geo_info {		/* bounding boxes of "info" file */
    long    bxl, bxr, byb, byt;
};

struct geo_info2 {		/* makebox/vln info of "info" file */
    long    nr_boxes, nr_groups;
};

struct geo_info3 {		/* makebox info of "info" file */
    long    bxl, bxr, byb, byt;	/* expansion window */
    long    nr_samples;		/* number of samples used */
};

struct geo_mc {			/* cell call information, "mc" file */
    char    inst_name[DM_MAXNAME + 1]; /* cell instance name */
    char    cell_name[DM_MAXNAME + 1]; /* cell name */
    long    imported;		/* local or imported flag */
    long    mtx[6];		/* orientation, scaling and translation */
    long    bxl, bxr, byb, byt;	/* bounding box inclusive repetition */
    long    dx, nx, dy, ny;	/* repetition parameters */
};

struct geo_lup {		/* link-up information, "lup" file */
    char    cell_name[DM_MAXNAME + 1]; /* cell name */
    long    count;		/* number of link-ups */
};

struct geo_box {		/* box information, "box" file */
    long    layer_no;		/* layer number */
    long    xl, xr, yb, yt;	/* co-ordinates */
    long    bxl, bxr, byb, byt;	/* bounding box inclusive repetition */
    long    dx, nx, dy, ny;	/* repetition parameters */
};

struct geo_term {		/* terminal information, "term" file */
    char    term_name[DM_MAXNAME + 1]; /* terminal name */
    long    layer_no;		/* layer number */
    long    xl, xr, yb, yt;	/* co-ordinates */
    long    bxl, bxr, byb, byt;	/* bounding box inclusive repetition */
    long    dx, nx, dy, ny;	/* repetition parameters */
};

struct geo_nor_ini {		/* non-orth element info, "nor" file */
    long    layer_no;		/* layer number */
    long    elmt;		/* element code */
    long    no_xy;		/* number of co-ordinate pairs */
    long    bxl, bxr, byb, byt;	/* bounding box exclusive repetition */
    long    r_bxl, r_bxr, r_byb, r_byt;	/* bounding box incl. repetition */
    long    nx, ny;		/* repetition counts */
    double  dx, dy;		/* repetition distances */
};

struct geo_nor_xy {		/* non-orth element info, "nor" file */
    double  x, y;		/* x and y co-ordinate or parameter */
};

struct geo_boxlay {		/* expanded boxes, "lay_bxx" file */
    long    xl, xr, yb, yt;	/* co-ordinates */
    long    chk_type;		/* check-type */
};

struct geo_termlay {		/* expanded terminals, "t_lay_bxx" file */
    long    xl, xr, yb, yt;	/* co-ordinates */
    long    term_number;	/* terminal number */
};

struct geo_nxx_ini {		/* expanded non-orth element, "lay_nxx" file */
    long    elmt;		/* element code */
    long    no_xy;		/* number of co-ordinate pairs */
/*
** If elmt-code is CIRCLE_NOR the following part contains
** the circle data.  The polygon by means of which the circle
** can be approximated is in this case stored in the geo_nxx_xy
** records that follow this geo_nxx_ini.
*/
    double  xc, yc;		/* centre co-ordinates */
    double  r1, r2;		/* outer and inner radius */
    double  a1, a2;		/* start and stop angle */
};

struct geo_nxx_xy {		/* expanded non-orth element, "lay_nxx" file */
    double  x, y;		/* x and y co-ordinate or parameter */
};

struct geo_vlnlay {		/* vertical line segments, "lay_vln" file */
    long    x;			/* x co-ordinate */
    long    yb, yt;		/* y-bottom and y-top values */
    char    occ_type;		/* occurrence type */
    char    con_type;		/* connect type */
    long    grp_number;		/* identifies part of a layer */
    long    chk_type;		/* check type */
};

struct geo_teq {		/* equivalence file, "teq" file */
    long    term_number, grp_number;
};

struct geo_tid {		/* terminal identifiers, "tid" file */
    long    term_offset;	/* terminal number */
    char    cell_name[DM_MAXNAME + 1]; /* originating cell */
    char    inst_name[DM_MAXNAME + 1]; /* instance name */
    char    term_name[DM_MAXNAME + 1]; /* terminal name */
    long    m_nx, m_ny;		/* cell repetition */
    long    t_nx, t_ny;		/* terminal repetition */
};

struct geo_spec {		/* special info for preplot, "spec" file */
    char    layer[DM_MAXLAY + 1]; /* layer name */
    long    xl, xr, yb, yt;	/* co-ordinates */
    char    name[3*DM_MAXNAME + 4]; /* terminal or cell id. string */
};

struct geo_floc {               /* fault location file */
    int     number;             /* fault number */
    int     type;               /* fault type */
    long    xl, xr, yb, yt;     /* co-ordinates */
    long    detect_time;        /* detection time */
};

struct geo_gln {		/* gln format coordinates */
    long xl, xr, yl, yr;
};

struct geo_anno_format {
    long fmajor;
    long fminor;
};

struct geo_anno_line {
    double x1;
    double y1;
    double x2;
    double y2;
    int mode;
};

struct geo_anno_text {
    double x;
    double y;
    char text[DM_MAXLINE];
    double ax, ay;
};

struct geo_anno_label {
    double x, y;
    double ax, ay;
    int maskno;
    char name[DM_MAXNAME + 1];
    char Class[DM_MAXNAME + 1];
    char Attributes[DM_MAXLINE];
};

struct geo_anno {
    int type;
    union {             /* to be extended with other elements */
        struct geo_anno_format format;
        struct geo_anno_line line;
        struct geo_anno_text text;
        struct geo_anno_label Label;
    } o; /* object */
};

struct cir_mc {			/* circuit call information, "mc" file */
    char    cell_name[DM_MAXNAME + 1]; /* cell name */
    char    inst_name[DM_MAXNAME + 1]; /* cell instance name */
    long    imported;		/* local or imported flag */
    char   *inst_attribute;	/* cell attributes */
    long    inst_dim;		/* no. lower/upper pairs */
    long   *inst_lower, *inst_upper;	/* lower and upper range */
};

struct cir_term {		/* terminal information, "term" file */
    char    term_name[DM_MAXNAME + 1]; /* terminal name */
    char   *term_attribute;	/* terminal attributes */
    long    term_dim;		/*  no. lower/upper pairs */
    long   *term_lower, *term_upper;	/*  lower and upper range */
};

struct cir_net {		/* circuit net information, "net" file */
    char    net_name[DM_MAXNAME + 1]; /* net name */
    char   *net_attribute;	/* net attributes */
    long    net_dim;		/* no. lower/upper pairs */
    long   *net_lower, *net_upper;	/*  lower and upper range */
    char    inst_name[DM_MAXNAME + 1]; /* instance name */
    long    inst_dim;		/* no. lower/upper pairs */
    long   *inst_lower, *inst_upper;	/*  lower and upper range */
    long    ref_dim;		/* no. lower/upper pairs */
    long   *ref_lower, *ref_upper;	/*  lower and upper range */
    long    net_neqv;		/* no. sub cir_net struct's */
    struct cir_net *net_eqv;	/* sub cir_net's */
};

struct cir_nethead {		/* head of circuit net */
    long    cd_nr;		/* distr.net number */
    long    node_nr;		/* net node number */
    long    node_x;		/* node x coord. */
    long    node_y;		/* node y coord. */
    long    net_neqv;		/* no. sub cir_net struct's */
    long long offset;		/* sub cir_net offset in "net" stream */
    int     lay_nr;		/* node layer number */
    short   area;		/* area flag */
    short   term;		/* term flag */
};

struct cir_sls {		/* "sls" file */
    char   *sls_buffer;		/* io buffer */
    long    sls_dim;		/* no. bytes in buffer */
};

struct cir_info {		/* "info" file */
    long    bxl, bxr, byb, byt;	/* circuit bounding box */
};

struct cir_graphic {		/* "graphic" file */
    char   *graphic_buffer;	/* io buffer */
    long    graphic_dim;	/* no. bytes in buffer */
};

struct cir_swift { /* the swift file "swift" */
  char	*swift_buffer;		/* io buffer for swift file */
  long	swift_dim; 		/* no. bytes in buffer */
};

struct cir_fault { /* the faults list file "fault" */
  long	number;			  /* Fault number */
  char	type[DM_MAXNAME+1];	  /* Fault type */
  char	*fault_attribute;	  /* Fault attributes */
  char  inst_name[DM_MAXNAME+1];  /* Instance name */
  long	inst_dim;		  /* no. lower/upper pairs */
  long  *inst_lower, *inst_upper; /* lower and upper range */
  long	ref_dim;		  /* no. lower/upper pairs */
  long  *ref_lower, *ref_upper;   /* lower and upper range */
  long  fault_neqv;		  /* no. sub cir_fault struct's */
  struct cir_fault *fault_eqv;	  /* sub cir_fault's */
};

struct flp_mc {			/* cell call information, "mc" file */
    char    inst_name[DM_MAXNAME + 1]; /* cell instance name */
    char    cell_name[DM_MAXNAME + 1]; /* cell name */
    long    imported;		/* local or imported flag */
    long    mtx[6];		/* orientation, scaling and translation */
    long    bxl, bxr, byb, byt;	/* bounding box inclusive repetition */
    long    dx, nx, dy, ny;	/* repetition parameters */
};

struct flp_info {		/* "info" file */
    long    bxl, bxr, byb, byt;	/* cell bounding box */
};

struct flp_term {		/* terminal information, "term" file */
    char    term_name[DM_MAXNAME + 1]; /* terminal name */
    char   *term_attribute;	/* terminal attributes */
    long    layer_no;		/* layer number */
    long    side;		/* sides terminal can be on */
    long    xl, xr, yb, yt;	/* terminal co-ordinates */
    long    bxl, bxr, byb, byt;	/* bounding box inclusive repetition */
    long    dx, nx, dy, ny;	/* repetition parameters */
};

struct flp_chan {		/* channel information, "chan" file */
    char    channel_name[DM_MAXNAME + 1];
    long    xl, yb, xr, yt;	/* channel position */
    long    kind;		/* channel type */
    long    order;		/* channel order */
    long    flp_nlist;		/* number of flp_netlists */
    struct flp_glr *flp_netlist;
};

struct flp_glr {		/* global routing nets */
    char    net_name[DM_MAXNAME + 1];
    char   *net_attribute;	/* net attributes */
    long    flp_nconnect;	/* number of flp_netconnects */
    struct flp_connect *flp_netconnect;
};

struct flp_connect {		/* the connects of a global routing net */
    char    connect_name[DM_MAXNAME + 1];
    long    connect_type;	/* the type of connect */
    char    connect_origin[DM_MAXNAME + 1];
				/* a cell instance name or a channel name */
    long    nx, ny;		/* repetition parameters */
};

extern struct geo_info		ginfo;
extern struct geo_info2		ginfo2;
extern struct geo_info3		ginfo3;
extern struct geo_box		gbox;
extern struct geo_mc		gmc;
extern struct geo_term		gterm;
extern struct geo_nor_ini	gnor_ini;
extern struct geo_nor_xy	gnor_xy;
extern struct geo_boxlay	gboxlay;
extern struct geo_termlay	gtermlay;
extern struct geo_vlnlay	gvlnlay;
extern struct geo_spec		gspec;
extern struct geo_teq		gteq;
extern struct geo_tid		gtid;
extern struct geo_lup		glup;
extern struct geo_nxx_ini	gnxx_ini;
extern struct geo_nxx_xy	gnxx_xy;
extern struct geo_floc          gfloc;
extern struct geo_gln		ggln;
extern struct geo_anno          ganno;

extern struct cir_mc		cmc;
extern struct cir_term		cterm;
extern struct cir_net		cnet;
extern struct cir_nethead	cnethead;
extern struct cir_sls		csls;
extern struct cir_info		cinfo;
extern struct cir_graphic	cgraphic;
extern struct cir_swift 	cswift;
extern struct cir_fault 	cfault;

extern struct flp_mc		fmc;
extern struct flp_info		finfo;
extern struct flp_term		fterm;
extern struct flp_chan		fchan;

extern char *dmpls;		/* process locking code */
extern char *dmhome;		/* home project */
extern char *dmcur_project;	/* current project */
extern char *dmcur_view;	/* current view */
extern char *dmcur_cell;	/* current cell */
extern char *dmprompt;		/* current prompt */
extern char *dmtoolbox;		/* design tools */

extern int   dm_extended_format;
extern int   dm_get_do_not_alloc;
extern int   dm_maxname;

extern char *dmviews[];		/* views that are supported */

typedef struct {
    char   *alias;		/* local alias name for cell */
    char   *cellname;		/* remote cell name */
    char   *dmpath;		/* net-path to database */
} IMPCELL;

typedef struct {
    int       procid;		/* process id when procpath != '\0' */
    char      procpath[DM_MAXPATHLEN];
                                /* process path when procid < 0 */
    double    lambda;		/* lambda value */
    int       n_samples;	/* number of samples per lambda */
    char     *dmpath;		/* path to database */
    char     *projectid;	/* project name */
    int	      mode;		/* project open mode */
    int       projectno;	/* project key index */
    int       release;          /* project release number */
    char    **celllist[DM_NOVIEWS];	/* a celllist per view */
    IMPCELL **impcelllist[DM_NOVIEWS];	/* an impcelllist per view */
    DM_PROCDATA *maskdata;	/* pointer to the process data */
} DM_PROJECT;

typedef struct {
    DM_PROJECT *dmproject;	/* project key */
    char       *cell;		/* cell name */
    char       *versionstatus;
    int         versionnumber;
    char       *view;		/* view name */
    int         mode;		/* cell checkout mode */
    int         keyno;		/* cell key index */
} DM_CELL;

typedef struct {
    DM_CELL   *dmkey;		/* cell key */
    FILE      *dmfp;		/* fopen file pointer */
    char      *stream;		/* stream name */
    char      *mode;		/* stream open mode */
    int        streamno;	/* stream key index */
} DM_STREAM;

extern char	icdpath[];

#include "src/libddm/dmproto.h"
#include "src/libddm/dmfuncs.h"

#endif /* __DMINCL_H */
