/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
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

#include "src/ocean/seadali/header.h"

extern  DM_PROJECT * dmproject;
extern int  new_cmd;
extern int ImageMode;
extern char Weedout_lib[];

struct modlist {
    char  **mname;
    int     nr_mod;
};

int   g_ask_nr;
char *g_ask_str[LIST_LENGTH + 4];

static struct modlist   mlst;
static struct modlist   alias_lst;

static int  add_mlst (int local_imported, char *m_name);
static void sort_mlst (int local_imported);

int read_modlist ()
{
    static int firsttime = TRUE; /* needed for re-read */
    register char **pmod;

    if (firsttime == TRUE) {
	MALLOCN (mlst.mname, char *, 50);
	if (!mlst.mname) {
	    fatal (0001, "read_modlist");
	    return (-1);
	}
	firsttime = FALSE;
    }

    pmod = (char **) dmGetMetaDesignData (CELLLIST, dmproject, LAYOUT);
    if (!pmod) return (-1);
    mlst.nr_mod = 0;
    while (*pmod) {
	if (!add_mlst (LOCAL, *pmod++)) {
	    fatal (0002, "read_modlist");
	    return (-1);
	}
    }
    sort_mlst (LOCAL);
    return (0);
}

int read_impclist ()
{
    static int firsttime = TRUE; /* needed for re-read */
    register IMPCELL **pmod, *ic;
    char *s;
    int weedout = (ImageMode == TRUE && strlen (Weedout_lib) > 0);

    if (firsttime == TRUE) {
	MALLOCN (alias_lst.mname, char *, 50);
	if (!alias_lst.mname) {
	    fatal (0001, "read_impclist");
	    return (-1);
	}
	firsttime = FALSE;
    }

    pmod = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, LAYOUT);
    if (!pmod) return (-1);
    alias_lst.nr_mod = 0;

    while ((ic = *pmod++)) {
	/* Patrick: skip names from primitives library */
	if (weedout && ic -> dmpath && (s = strrchr (ic -> dmpath, '/'))) {
	    if (strcmp (s+1, Weedout_lib) == 0) continue;
	}
	if (!add_mlst (IMPORTED, ic -> alias)) {
	    fatal (0002, "read_impclist");
	    return (-1);
	}
    }
    sort_mlst (IMPORTED);
    return (0);
}

static int add_mlst (int local_imported, char *m_name)
{
    struct modlist *lst_p;
    int nr;

    lst_p = (local_imported == LOCAL) ? &mlst : &alias_lst;
    nr = lst_p -> nr_mod;

    if (nr % 50 == 0 && nr > 0) {
	if (!(lst_p -> mname = (char **) realloc ((char *) lst_p -> mname, (nr + 50) * sizeof (char *)))) {
	    ptext ("No memory available!");
	    return (FALSE);
	}
    }
    lst_p -> mname[nr] = m_name;
    lst_p -> nr_mod++;
    return (TRUE);
}

int compareName (const void *n1, const void *n2)
{
    char **s1 = (char **) n1;
    char **s2 = (char **) n2;
    return (strcmp (*s1, *s2));
}

static void sort_mlst (int local_imported)
{
    struct modlist *lst_p;

    lst_p = (local_imported == LOCAL) ? &mlst : &alias_lst;

    qsort (lst_p -> mname, lst_p -> nr_mod, sizeof (char *), compareName);
}

char * ask_cell (int local_imported)
{
    static int  nl, ni, pnl, pni;
    char    err_str[MAXCHAR];
    char    tmp_str[DM_MAXNAME+1];
    char  **c_list;
    char   *c_name = NULL;
    register int i, j, k, nr_cells;
    int     f, g, i_old;

    if (local_imported == LOCAL) {
	nr_cells = mlst.nr_mod;
	c_list = mlst.mname;
	i = nl; i_old = pnl;
    }
    else {
	nr_cells = alias_lst.nr_mod;
	c_list = alias_lst.mname;
	i = ni; i_old = pni;
    }

    if (nr_cells <= 0) {
	sprintf (err_str, "No %s cell names!",
	    (local_imported == LOCAL ? "local" : "imported"));
	ptext (err_str);
	return (NULL);
    }

    while (i >= nr_cells) i -= LIST_LENGTH;
    while (i_old >= nr_cells) i_old -= LIST_LENGTH;

    f = 0;
    g_ask_str[0] = "-return-";
    if (nr_cells > LIST_LENGTH) {
	g_ask_str[1] = "-keyboard-";
	g_ask_str[2] = "-next-";
	g_ask_str[3] = "-prev-";
	f = 3;
    }

    for (;;) {
ask1:
	if (new_cmd == -1) {
	    if ((k = i + LIST_LENGTH) > nr_cells) k = nr_cells;
	    for (j = i; j < k; ++j)
		g_ask_str[k - j + f] = c_list[j];
	    j = ask (k - i + f + 1, g_ask_str, (int) -1);
	}
	else { /* only by do_add_inst () */
	    j = new_cmd;
	    new_cmd = -1;
	    pre_cmd_proc (j, g_ask_str);
	}

	if (j > f) { /* cellname */
	    c_name = g_ask_str[j];
	    if (dmTestname (c_name) < 0) {
		sprintf (err_str, "Bad cell name '%s'!", c_name);
		ptext (err_str);
		c_name = NULL;
	    }
	    g_ask_nr = j;
	    goto ret;
	}

	if (j <= 0) {
	    ptext ("");
	    goto ret;	/* return */
	}

	if (j == 3) {		/* prev */
	    if ((i -= LIST_LENGTH) < 0) { /* goto end */
		i = 0;
		while (i + LIST_LENGTH < nr_cells) i += LIST_LENGTH;
	    }
	}
	else if (j == 2) {	/* next */
	    if ((i += LIST_LENGTH) >= nr_cells) i = 0; /* begin */
	}
	else {			/* keyboard */
	    j = ask_name ("cellname: ", tmp_str, FALSE);
	    post_cmd_proc (1, g_ask_str);
	    ptext ("");
	    if (j) goto ask1; /* no name, return given! */
	    g = strlen (tmp_str);
	    if (g < 3 && tmp_str[g-1] == '*') { /* set new position */
		if (g == 1) { /* goto previous position! */
		    g = i; i = i_old; i_old = g;
		}
		else {
set_position:
		    j = k = 0;
		    do {
			g = k;
			if ((k += LIST_LENGTH) > nr_cells) break;
			for (; j < k; ++j)
			    if (*tmp_str <= *c_list[j]) {
				k = nr_cells; /* force exit while loop */
				break;
			    }
		    } while (k < nr_cells);
		    if (g != i) { i_old = i; i = g; }
		}
		goto ask1;
	    }
	    else if (dmTestname (tmp_str) < 0) {
		sprintf (err_str, "Bad cell name '%s'!", tmp_str);
		ptext (err_str);
		if (isalpha ((int)*tmp_str)) goto set_position;
		goto ask1;
	    }
	    j = k = 0;
	    do {
		g = k;
		if ((k += LIST_LENGTH) > nr_cells) k = nr_cells;
		for (; j < k; ++j)
		    if (!strcmp (tmp_str, c_list[j])) {
			if (g != i) { i_old = i; i = g; }
			c_name = c_list[j];
			g_ask_nr = -1;
			goto ret;
		    }
	    } while (k < nr_cells);
	    sprintf (err_str, "Unknown cell name '%s'!", tmp_str);
	    ptext (err_str);
	    goto set_position;
	}
    }

ret:
    if (local_imported == LOCAL) {
	nl = i; pnl = i_old;
    }
    else {
	ni = i; pni = i_old;
    }
    return (c_name);
}
