/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
 *	H.T. Fassotte
 *	S. de Graaf
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

#include "src/dali/header.h"

extern DM_PROJECT *dmproject;
extern int  erase_text;
extern int  new_cmd;
extern int  ask_again;

struct modlist {
    char **mname;
    int    nr_mod;
};

static struct modlist mlst;
static struct modlist alias_lst;

static int add_mlst (int local_imported, char *m_name);

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
    return;
}

int read_modlist ()
{
    static int Firsttime = 1; /* needed for re-read */
    register char **pmod;

    if (Firsttime) {
	MALLOCN (mlst.mname, char *, 50);
	if (!mlst.mname) {
	    fatal (0001, "read_modlist");
	    return (-1);
	}
	Firsttime = 0;
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
    static int Firsttime = 1; /* needed for re-read */
    register IMPCELL **pmod;

    if (Firsttime) {
	MALLOCN (alias_lst.mname, char *, 50);
	if (!alias_lst.mname) {
	    fatal (0001, "read_impclist");
	    return (-1);
	}
	Firsttime = 0;
    }

    pmod = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, LAYOUT);
    if (!pmod) return (-1);
    alias_lst.nr_mod = 0;
    while (*pmod) {
	if (!add_mlst (IMPORTED, (*pmod) -> alias)) {
	    fatal (0002, "read_impclist");
	    return (-1);
	}
	++pmod;
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
	if (!(lst_p -> mname = (char **) realloc (lst_p -> mname, (nr + 50) * sizeof (char *)))) {
	    ptext ("No memory available!");
	    return (FALSE);
	}
    }
    lst_p -> mname[nr] = m_name;
    lst_p -> nr_mod++;
    return (TRUE);
}

char * ask_cell (int local_imported)
{
    static char *ask_str[LIST_LENGTH + 4];
    static int  nl, ni, ask_nr = -1;
    char    err_str[MAXCHAR];
    char    tmp_str[DM_MAXNAME+1];
    char  **c_list;
    char   *c_name = NULL;
    int     i, j, k, n, nr_cells;
    int     f, g, rv;

    if (!ask_again) post_cmd_proc (ask_nr);

    if (local_imported == LOCAL) {
	nr_cells = mlst.nr_mod;
	c_list = mlst.mname;
	i = nl;
    }
    else {
	nr_cells = alias_lst.nr_mod;
	c_list = alias_lst.mname;
	i = ni;
    }

    if (nr_cells <= 0) {
	sprintf (err_str, "No %s cell names!",
	    (local_imported == LOCAL ? "local" : "imported"));
	ptext (err_str);
	return (NULL);
    }
    if (ask_again < 2) {
	sprintf (err_str, "Select %s cell name!",
	    (local_imported == LOCAL ? "a local" : "an imported"));
	ptext (err_str);
    }

    while (i >= nr_cells) i -= LIST_LENGTH;

    f = 0;
    ask_str[0] = "-cancel-";
    if (nr_cells > LIST_LENGTH) {
	ask_str[1] = "-keyboard-";
	ask_str[2] = "-next-";
	ask_str[3] = "-prev-";
	f = 3;
    }

    for (;;) {
	if (new_cmd < 0) {
	    if ((k = i + LIST_LENGTH) > nr_cells) k = nr_cells;
	    for (j = i; j < k; ++j)
		ask_str[k - j + f] = c_list[j];
	    j = ask (k - i + f + 1, ask_str, ask_again == 2 ? ask_nr : -1);
	}
	else { /* only by do_add_inst () */
	    post_cmd_proc (ask_nr);
	    j = new_cmd;
	    new_cmd = -1;
	    pre_cmd_proc (j);
	}

	if (j > f) { /* cellname */
	    if (ask_again) ask_again = 2;
	    c_name = ask_str[ask_nr = j];
	    goto ret;
	}

	if (j <= 0) {
	    erase_text = 1;
	    ask_nr = -1;
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
	else {/* j == 1 */	/* keyboard */
	    char *s;
	    int g2, star_on_end;
	    g2 = g = ask_name ("cellname: ", tmp_str, FALSE);
	    post_cmd_proc (j);
	    ptext ("");
	    while (g && tmp_str[g-1] == '*') { --g; tmp_str[g] = 0; }
	    if (!g) goto ask1; /* no name, return given! */
	    star_on_end = (g != g2);
	    if ((s = strchr (tmp_str, '*'))) {
		*s++ = 0;
		g = strlen (tmp_str);
		while (*s == '*') ++s;
		g2 = strlen (s);
	    }
	    for (j = 0; j < nr_cells; ++j) {
		if ((rv = strncmp (tmp_str, c_list[j], g)) <= 0) { /* found */
		    i = j / LIST_LENGTH * LIST_LENGTH;
		    if (rv < 0) goto ask1;
		    if (s) { /* does end or part of cell also match? */
			if ((n = strlen (c_list[j]) - g2) < g) continue;
			for (k = star_on_end ? g : n; k < n; ++k) {
			    if (!strncmp (s, c_list[j] + k, g2)) break;
			}
			if (strncmp (s, c_list[j] + k, g2)) continue;
		    }
		    c_name = c_list[j];
		    /* set new menu and do pre_cmd_proc */
		    if ((k = i + LIST_LENGTH) > nr_cells) k = nr_cells;
		    for (n = i; n < k; ++n) ask_str[k - n + f] = c_list[n];
		    menu (k - i + f + 1, ask_str);
		    pre_cmd_proc (ask_nr = k - j + f);
		    if (ask_again) ask_again = 2;
		    goto ret;
		}
	    }
	}
ask1:
	if (ask_again) { ask_again = 1; c_name = ask_str[0]; goto ret; }
    }

ret:
    if (local_imported == LOCAL) { nl = i; } else { ni = i; }
    return (c_name);
}
