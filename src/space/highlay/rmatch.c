/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include "src/space/highlay/incl.h"

static int currLevel;
static int setLevel;

#ifdef __cplusplus
extern "C" {
#endif
Private int testNumber (char *list, char *member);
Private void setNestLevel (FILE *fp, int change);
Private char *getKeyword (FILE *fp);
Private char *getWord (FILE *fp);
Private char *getToken (FILE *fp);
Private int getFirstNonSpace (FILE *fp);
Private int myGetc (FILE *fp);
Private void myUngetc (int c, FILE *fp);
#ifdef __cplusplus
}
#endif

void readMatchOutput (DM_CELL *cirKey)
{
    struct net_ref *new_net_ref, *search_ref, *prev_ref;
    DM_STREAM * dms;
    FILE * fp;
    char *keyw, *name;
    int thisResultGroup, thisItem, len;

    if ((dms = dmOpenStream (cirKey, "match", "r")) == NULL) {
	say ("cannot read match results\n");
	die ();
    }

    fp = dms -> dmfp;

    currLevel = 0;
    setLevel = 0;

    setNestLevel (fp, 1);
    while ((keyw = getKeyword (fp))) {
	thisResultGroup = 0;
	if (optMatching && strsame (keyw, "matching")) {
	    thisResultGroup = 1;
	}
	if (optInconclusive && strsame (keyw, "inconclusive")) {
	    thisResultGroup = 1;
	}
	if (optDeficient && strsame (keyw, "deficient")) {
	    thisResultGroup = 1;
	}

	if (thisResultGroup) {
	    setNestLevel (fp, 1);
	    while ((keyw = getKeyword (fp))) {
		thisItem = 0;
		if (strsame (keyw, "nets")) {
		    if (netGroups && testNumber (netGroups, getWord (fp)))
			thisItem = 1;
		    else if (optNets)
			thisItem = 1;
		}
		if (strsame (keyw, "ports")) {
		    if (portGroups && testNumber (portGroups, getWord (fp)))
			thisItem = 1;
		    else if (optPorts)
			thisItem = 1;
		}
		if (strsame (keyw, "instances")) {
		    if (devGroups && testNumber (devGroups, getWord (fp)))
			thisItem = 1;
		    else if (optDevices)
			thisItem = 1;
		}

		if (thisItem) {
		    setNestLevel (fp, 1);
		    getKeyword (fp);
		    getKeyword (fp);

		    while ((name = getWord (fp))) {

			new_net_ref = NEW (struct net_ref, 1);
			len = strlen (name) + 1;
			new_net_ref -> name = NEW (char, len);
			strcpy (new_net_ref -> name, name);

			if (Begin_net_ref) {
			    search_ref = Begin_net_ref;
			    prev_ref = NULL;
			    while (search_ref && strcmp (search_ref -> name, name) < 0) {
				prev_ref = search_ref;
				search_ref = search_ref -> next;
			    }
			    if (search_ref && strsame (search_ref -> name, name)) {
				/* double; skip it */
				DISPOSE (new_net_ref -> name, len);
				DISPOSE (new_net_ref, sizeof(struct net_ref));
			    }
			    else if (prev_ref) {
				new_net_ref -> next = prev_ref -> next;
				prev_ref -> next = new_net_ref;
			    }
			    else {
				new_net_ref -> next = Begin_net_ref;
				Begin_net_ref = new_net_ref;
			    }
			}
			else {
			    Begin_net_ref = new_net_ref;
			    new_net_ref -> next = NULL;
			}
		    }

		    setNestLevel (fp, -1);
		}
	    }
	    setNestLevel (fp, -1);
	}
    }

    dmCloseStream (dms, COMPLETE);
}

Private int testNumber (char *list, char *member)
{
    char *p;
    int l = strlen (member);

    for (p = list; *p; p++) {
	if (strncmp (p, member, l) == 0
	    && (p[l] == ' ' || p[l] == ',' || p[l] == '\0'))
	    return (1);
    }

    return (0);
}

Private void setNestLevel (FILE *fp, int change)
{
    setLevel += change;

    while (currLevel != setLevel && getToken (fp));
}

Private char *getKeyword (FILE *fp)
{
    char *w;

    setNestLevel (fp, 0);

    while ((w = getToken (fp)) && w[0] != '(' && w[0] != ')');

    if (w[0] == '(')
	w = getToken (fp);
    else if (w[0] == ')')
	w = NULL;

    return (w);
}

Private char *getWord (FILE *fp)
{
    char *w = getToken (fp);

    if (w && (w[0] == '(' || w[0] == ')')) {
	return (NULL);
    }

    return (w);
}

Private char *getToken (FILE *fp)
{
    static char buf[132];
    int c = getFirstNonSpace (fp);
    int c2;
    int i;

    if (c == EOF)
	return (NULL);
    else if (c == '(' || c == ')') {
	buf[0] = c;
	buf[1] = '\0';
    }
    else {
	i = 0;
	while (c != '(' && c != ')'
	       && c != ' ' && c != '\t' && c != '\n' && c != EOF) {
	    if (c == '/') {
		if ((c2 = myGetc (fp)) == '/') {
		    while ((c2 = getc (fp)) != '\n' && c2 != EOF); /* comment */
		    break;
		}
		else {
		    myUngetc (c2, fp);
		}
	    }
	    buf[i++] = (char)c;
	    c = myGetc (fp);
	}
	buf[i] = '\0';
    }

    return (buf);
}

Private int getFirstNonSpace (FILE *fp)
{
    int c, c2;

    while ((c = myGetc (fp)) != EOF
	   && (c == ' ' || c == '\n' || c == '\t' || c == '/')) {
	if (c == '/') {
	    if ((c2 = myGetc (fp)) == '/') {
		while ((c2 = getc (fp)) != '\n' && c2 != EOF);  /* comment */
		if (c2 == EOF) {
		    c = c2;
		    break;
		}
	    }
	    else {
		myUngetc (c2, fp);
		break;
	    }
	}
    }

    return (c);
}

Private int myGetc (FILE *fp)
{
    int c = getc (fp);
	 if (c == '(') ++currLevel;
    else if (c == ')') --currLevel;
    return (c);
}

Private void myUngetc (int c, FILE *fp)
{
	 if (c == '(') --currLevel;
    else if (c == ')') ++currLevel;
    ungetc (c, fp);
}
