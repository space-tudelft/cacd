#include <stdio.h>

#define BUFLEN 512
char buf[BUFLEN];
char section[20];

char *argv0 = "getNAME";

int main (int argc, char *argv[])
{
    FILE *fp;
    char *s;
    int   c, i, p;

    if (argc <= 1) {
	fprintf (stderr, "\nUsage: %s file...\n\n", argv0);
	return (1);
    }

    while (*(++argv))
    {
	if (!(fp = fopen (*argv, "r"))) {
	    fprintf (stderr, "%s: cannot open file: %s\n", argv0, *argv);
	    return (1);
	}

	for (;;) {
	    if (!fgets (buf, BUFLEN, fp)) {
		fprintf (stderr, "%s: cannot find .TH in file: %s\n", argv0, *argv);
		return (1);
	    }
	    if (buf[0] == '.' && buf[1] == 'T' && buf[2] == 'H') break;
	}

	*section = 0;
	sscanf (buf, "%*s%*s%s", section);
	if (!*section) {
	    fprintf (stderr, "%s: no section in file: %s\n", argv0, *argv);
	    return (1);
	}

	for (;;) {
	    if (!fgets (buf, BUFLEN, fp)) {
		fprintf (stderr, "%s: cannot find .SH in file: %s\n", argv0, *argv);
		return (1);
	    }
	    if (buf[0] == '.' && buf[1] == 'S' && buf[2] == 'H') break;
	}

	p = i = 0;
	while ((c = fgetc (fp)) != '\n' && c != EOF) /* last line */
	{
	    if (c == '\\' && (c = fgetc (fp)) != '-') {
		if (c == 'f') { c = fgetc (fp);
		  //fprintf (stderr, "%s: %s: skip \\f%c\n", argv0, *argv, c);
		} else {
		    fprintf (stderr, "%s: %s: skip \\%c\n", argv0, *argv, c);
		}
	    }
	    else {
		if (c == '\t' || c == '\r') c = ' ';
		if (c == '-' && *section) {
		    buf[i++] = '(';
		    s = section; while (*s) buf[i++] = *s++;
		    buf[i++] = ')';
		    buf[i++] = ' ';
		    while (i < 18) buf[i++] = ' ';
		    buf[i] = '\0';
		    printf ("%s", buf);
		    i = 0; *section = 0;
		}
		if (c == ' ' && c == p) ;
		else  buf[i++] = p = c;
	    }
	}
	if (p == ' ') --i;
	buf[i] = '\0';
	printf ("%s\n", buf);
	fclose (fp);
    }
    return (0);
}
