/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Initial lexer.
 */

#include "spice.h"
#include "cpdefs.h"
#include <errno.h>

#include <sys/ioctl.h>

#include <unistd.h>

#include "fteinput.h"

FILE *cp_inp_cur = NULL;
int cp_event = 1;
bool cp_interactive = true;
bool cp_bqflag = false;
char *cp_promptstring = NULL;
char *cp_altprompt = NULL;

static int numeofs = 0;

static void prompt();
static int  input (FILE *fp);
extern void Input();

#define ESCAPE '\033'

/* Return a list of words, with backslash quoting and '' quoting done.
 * Strings enclosed in "" or `` are made single words and returned,
 * but with the "" or `` still present. For the \ and '' cases, the
 * 8th bit is turned on (as in csh) to prevent them from being recogized,
 * and stripped off once all processing is done. We also have to deal with
 * command, filename, and keyword completion here.
 * If string is non-NULL, then use it instead of the fp. Escape and EOF
 * have no business being in the string.
 */

#define newword \
	buf[i] = '\0'; cw->wl_word = copy(buf); \
        cw->wl_next = alloc(wordlist); \
        cw->wl_next->wl_prev = cw; \
        cw = cw->wl_next; \
        cw->wl_next = NULL; \
        i = 0;

wordlist * cp_lexer (char *string)
{
    wordlist *wlist = NULL, *cw = NULL;
    char buf[BSIZE_SP], linebuf[BSIZE_SP];
    int c, d, i, j, paren;

    if (!cp_inp_cur) cp_inp_cur = cp_in;

    if (!string && cp_interactive) {
        cp_ccon(true);
        prompt();
    }
nloop:
    i = j = 0;
    paren = 0;
    wlist = cw = alloc(wordlist);
    cw->wl_next = cw->wl_prev = NULL;
    for (;;) {
        if (string) {
            c = *string++;
            if (c == ESCAPE) c = '[';
        } else
            c = input(cp_inp_cur);

gotchar:
	if (c != EOF && c != ESCAPE) linebuf[j++] = c;
        if (c != EOF) { numeofs = 0;
	    c = strip(c); /* Don't need to do this really. */
	}
        if (i == BSIZE_SP - 1) {
            fprintf(cp_err, "Warning: word too long.\n");
            c = ' ';
        }
        if (j == BSIZE_SP - 1) {
            fprintf(cp_err, "Warning: line too long.\n");
            if (cp_bqflag)
                c = EOF;
            else
                c = '\n';
        }
        if (c == '\\' || c == '\026' /* ^V */) {
            c = (string ? *string++ : input(cp_inp_cur));
	    if (c) {
		linebuf[j++] = c;
		buf[i++] = quote(c);
		continue;
	    }
        }
        if (c == '\n' && cp_bqflag) c = ' ';
        if (c == EOF  && cp_bqflag) c = '\n';
        if (c == '*' && !cp_interactive && j == 1) { /* SdeG. comment */
            if (string) return (NULL);
            while ((c = input(cp_inp_cur)) != '\n' && c != EOF);
            goto nloop;
        }

        if (c == '(' || c == '[' || c == '{') /* MW. Nedded by parse() */
	    paren++;
	else if (c == ')' || c == ']' || c == '}')
	    paren--;

	d = 0;
        switch (c) {
	case ' ':
	case '\t':
            if (i) {
                newword;
            }
            break;

	case '\0':
	    if (!string) break;
	case '\n':
            if (i) {
                buf[i] = '\0';
                cw->wl_word = copy(buf);
            } else if (cw->wl_prev) {
                cw->wl_prev->wl_next = NULL;
                tfree(cw);
            } else {
                cw->wl_word = NULL;
            }
            goto done;

	case '\'':
            d = c;
            while ((c = (string ? *string++ : input(cp_inp_cur))) != d && i < BSIZE_SP - 1) {
                if (c == '\n' || c == EOF || c == ESCAPE) goto gotchar;
		buf[i++] = quote(c);
		linebuf[j++] = c;
            }
            linebuf[j++] = d;
            break;

	case '"':
	case '`':
            d = c;
	    if (d == '`') buf[i++] = d;
            while ((c = (string ? *string++ : input(cp_inp_cur))) != d && i < BSIZE_SP - 2) {
                if (c == '\n' || c == EOF || c == ESCAPE) goto gotchar;
                if (c == '\\') {
                    linebuf[j++] = c;
                    c = (string ? *string++ : input(cp_inp_cur));
		    buf[i++] = quote(c);
                }
		else
		    buf[i++] = c;
		linebuf[j++] = c;
            }
	    if (d == '`') buf[i++] = d;
            linebuf[j++] = d;
            break;

	case '\004':
	case EOF:
            if (cp_interactive && !cp_nocc && !string) {
                if (j == 0) {
                    if (cp_ignoreeof && numeofs++ < 23) {
                        fputs("Use \"quit\" to quit.\n", stdout);
                    } else {
                        fputs("quit\n", stdout);
                        cp_doquit();
                    }
                    goto done;
                }
		buf[i] = '\0';
                cp_ccom(wlist, buf, false);
                wl_free(wlist);
                fputc('\r', cp_out);
                prompt();
                for (i = 0; i < j; i++)
                    (void) ioctl(fileno(cp_out), TIOCSTI, linebuf + i);
                goto nloop;
            }
	    /* EOF during a source */
	    if (cp_interactive) {
		fputs("quit\n", stdout);
		cp_doquit();
		goto done;
	    }
	    return (NULL);
	case ESCAPE:
            if (cp_interactive && !cp_nocc) {
                fputs("\b\b  \b\b\r", cp_out);
                prompt();
	    if (i) {
		buf[i] = '\0';
                for (i = 0; i < j; i++)
                    (void) ioctl(fileno(cp_out), TIOCSTI, linebuf + i);
                cp_ccom(wlist, buf, true);
            }
                wl_free(wlist);
                goto nloop;
            }
	    /* Else fall through */
	case ',':
	    if (paren < 1 && i) {
		newword;
		break;
	    }
	    buf[i++] = c;
	    break;
	case '<':
	case '>':
	case ';':
	case '&':
            /* We have to remember the special case $< here */
            if (i && (c != '<' || buf[i-1] != '$')) {
		newword;
	    }
	    if (!i || c != '<' || buf[i-1] != '$') d = 1;
	default:
            buf[i++] = c;
	    if (d) {
		newword;
	    }
        }
    }
done:
    return (wlist);
}

static void prompt()
{
    char *s = cp_altprompt ? cp_altprompt : (cp_promptstring ? cp_promptstring : "-> ");

    for (; *s; s++) {
	if (*s == '!')
		fprintf(cp_out, "%d", cp_event);
	else	fputc(*s, cp_out);
    }
    fflush(cp_out);
}

/* A special 'getc' so that we can deal with ^D properly. There is no way for
 * stdio to know if we have typed a ^D after some other characters, so
 * don't use buffering at all
 */
int inchar (FILE *fp)
{
    char c;
    int i;

    if (cp_interactive && !cp_nocc) {
	do {
	    i = read((int) fileno(fp), &c, 1);
	} while (i == -1 && errno == EINTR);
	if (i == 0 || c == '\004') return (EOF);
	if (i == -1) { perror("read"); return (EOF); }
	return ((int) c);
    }
    i = getc(fp);
    return (i);
}

static int input (FILE *fp)
{
    REQUEST request;
    RESPONSE response;

    request.option = char_option;
    request.fp = fp;
    Input(&request, &response);
    return(response.reply.ch);
}
