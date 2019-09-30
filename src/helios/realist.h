/*
 * ISC License
 *
 * Copyright (C) 1995-2018 by
 *	Xianfeng Ni
 *	Ulrich Geigenmuller
 *	Simon de Graaf
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

#ifndef _realist_h
#define _realist_h

#define TRUE	1
#define FALSE	0
#define HUGE_INT_VALUE 30000
#define HUGE_FLOAT_VALUE 1.e30

#define MAX_LINE_LENGTH		200
#define MAXLENGTH_OF_FILENAME	100
#define MAX_BUFSIZE		20480	/* 20k */

/* Added by AvG 29-05-1997 */

typedef struct {
    int value;
    int cdflt;
    int udflt;
    int fdflt;
    int gdflt;
} bool_param_t;

typedef struct {
    int value;
    int cdflt;
    int udflt;
    int fdflt;
    int gdflt;
    int min;
    int max;
} int_param_t;

typedef struct {
    int value;
    int cdflt;
    int udflt;
    int fdflt;
    int gdflt;
    int min;
    int max;
    Widget *widgetChoice;
} intc_param_t;

typedef struct {
    float value;
    float cdflt;
    float udflt;
    float fdflt;
    float gdflt;
    float min;
    float max;
} float_param_t;

typedef struct {
    char *value;
    char *cdflt;
    char *udflt;
    char *fdflt;
    char *gdflt;
} str_param_t;

typedef struct {
    int value;
    int cdflt;
    int udflt;
    int fdflt;
    int gdflt;
    int nr;
    char **restrictedChoice;
    Widget *widgetChoice;
} strc_param_t;

typedef struct {
    int index;		/* index of the process */
    char *name;		/* name of the process */
    float lambda;	/* lambda of this process */
    char *comment;	/* comment about this process */
} Process;

#endif
