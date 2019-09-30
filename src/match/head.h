/* rcsid = "$Id: head.h,v 1.1 2018/04/30 12:17:30 simon Exp $" */
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
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
/*
 * Header file.
 * Contains all application dependent (macro) definitions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/match/mod.h"
#include "src/match/macro.h"

#define         MATCH_TOOLNAME  "match"
#define         MATCH_VERSION   "4.2.3"

#define		TERMINAL	001
#define		NET		002
#define		DEVICE		003
#define         SUBTERM         000

#define		UNDEFINED	000
#define		PRIMITIVE	001
#define		COMPOUND	002

#define		PASSIVE		000
#define		ACTIVE		004
#define		INVALID		010
#define		BOUND		014

#define		PARM_MAJOR	1
#define		INST_MAJOR	2

#define		SAVE		1
#define		NOSAVE		2

#define		ALL		0
#define		FULL		-1

#define		HEAD		1
#define		TAIL		2

#define		ERROR		-1

#define		BYMBN		1
#define		MKMBN		2

#define		LR		1
#define		UR		2
#define		UL		3
#define		LL		4
#define		CL		5
#define		HL		6
#define		VL		7
#define		LT		8
#define		TR		9
#define		BT		10
#define		TT		11

#define Set_field(object,field,value) (object->flags &= (~(field))); (object->flags |= (value))
#define Get_field(object,field) (object->flags & (field))
#define Set_flag(object,flag)   (object->flags |= (flag))
#define Clr_flag(object,flag)   (object->flags &= (~(flag)))
#define Get_flag(object,flag)   (object->flags & (flag))

#define		TYPE		0003
#define		STATE		0014
#define		TOUCHED		0020
#define		BOUNDED		0040
#define		MARKED		0100
#define		VISITED		0200
#define		CONNECTED	0200
#define		ISMBN		0400

#define		DEFAULT_SLS_LIB		"./std_lib"
#define		DEFAULT_PRIM_FILE	"match_prim"

typedef struct network {

	string		name;		/* name of the network */
	struct object	*thead;		/* head of terminal-list */
	struct object	*ttail;		/* tail of terminal-list */
	struct object	*nhead;		/* head of net-list */
	struct object	*ntail;		/* tail of net-list */
	struct object	*dhead;		/* head of device-list */
	struct object	*dtail;		/* tail of device-list */
	struct partition *part;		/* partition of ThiS network */
	unsigned long	color;		/* color of ThiS group */
	short 	n_terms;
	short	flags;

} network;


typedef struct object {

    string		name;		/* name of the object */
    struct network	*netw;		/* pointer to 'home' network */
    struct network	*call;		/* pointer to called network */

    struct link_type	*head;			/* head of neighbour list */
    struct link_type	*tail;			/* tail of neighbour list */

    struct object	*next_obj;	/* next object in net/dev list */

    struct object	*next_elm;	/* next element in element list */

    struct object	*equiv;		/* pointer to equivalent object */
					/* used to represent net statement */
    struct block	*block;		/* home block */
    struct bucket	*par_list;	/* pointer to parameter list */
    unsigned long	color;		/* color of the object */
    unsigned long	edges;		/* edge combination */
    int  		instance;	/* instance number */
    short		flavor;
    short		flags;		/* type (NET, TERM, ...) */

} object;


typedef struct link_type {

	struct link_type	*next_up;	/* pointer to next link in 'uplist' */
	struct link_type	*next_down;	/* idem 'downlist' */
	struct object	*net;		/* pointer to 'home' net */
	struct object	*dev;		/* pointer to 'home' device */
	struct object   *port;		/* device port (terminal) */
	unsigned long	color;		/* edge color */

} link_type;


typedef struct bucket {
	struct bucket *next, *prev;
	string key;
	void  *data;
} bucket;

typedef bucket *stack;
typedef bucket *queue;

typedef struct hash {
	struct bucket **table;
	int size;
	int bucks;
} hash;

typedef struct pair {
	long first, last, index;
	struct pair *next;
} pair;

typedef struct list {
	struct list *cdr;
	struct list *car;
} list;

typedef struct block {

	struct	block *next;	 /* pointer to next block */
	struct	block *prev;	 /* pointer to prev block */
	long	n_el;		 /* number of elements */
	object	*head;		 /* head of element list */
	struct	block *t_nxt;	 /* next pointer in touch list */
	struct	partition *part; /* home partition */
	struct	block **childs;	 /* pointer to child list */
	struct  block * parent;
	short	n_childs;	 /* number of childs in list */
	short	flags;
	short	level;

} block;


typedef struct partition {

 long n_blcks;	/* number of blocks in partition */
 long n_elemts;	/* number of elements in partition */
 long n_active;
 long n_passive;
 long n_bound;
 long n_invalid;
 long n_touched;
 long n_iter;
	block	*active;
	block	*passive;
	block	*invalid;
	block	*touched;
	block	*hist;		/* history of refinement process */

} partition;

typedef struct range {
	char * device;
	char * parameter;
	double value;
	struct range * next;
} range;

#include "src/match/type.h"
