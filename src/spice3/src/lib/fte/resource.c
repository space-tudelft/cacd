/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Resource-related routines.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"

#include <sys/types.h>

#include <sys/time.h>
#include <sys/resource.h>

#ifndef HAS_BSDRUSAGE
#include <sys/times.h>
#include <sys/param.h>
#endif

char *startdata;
char *enddata;
char *sbrk();

static void printres();
static void *baseaddr();

void init_rlimits()
{
    startdata = (char *) baseaddr();
    enddata = sbrk(0);
}

void init_time()
{
}

void com_rusage (wordlist *wl)
{
    /* Fill in the SPICE accounting structure... */

    if (wl && (eq(wl->wl_word, "everything") || eq(wl->wl_word, "all"))) {
        printres((char *) NULL);
    } else if (wl) {
	char *s;
        for (; wl; wl = wl->wl_next) {
            s = cp_unquote(wl->wl_word);
            printres(s);
            txfree(s);
            if (wl->wl_next) putc('\n', cp_out);
        }
    } else {
        printres("cputime");
        putc('\n', cp_out);
        printres("totalcputime");
        putc('\n', cp_out);
        printres("space");
    }
}

/* Find out if the user is approaching his maximum data size. */

void ft_ckspace()
{
    long usage, limit;
    static long old_usage = 0;
    struct rlimit rld;

    getrlimit(RLIMIT_DATA, &rld);
    if (rld.rlim_cur == RLIM_INFINITY) return;
    limit = rld.rlim_cur - (enddata - startdata); /* rlim_max not used */

    usage = sbrk(0) - enddata;

    if (limit < 0) return; /* what else do you do? */

    if (usage <= old_usage) return;

    old_usage = usage;

    if (usage > limit * 0.9) {
        fprintf(cp_err, "Warning - approaching max data size: ");
        fprintf(cp_err, "current size = %lu, limit = %lu.\n", usage, limit);
    }
}

/* Print out one piece of resource usage information. */

static void printres (char *name)
{
    bool yy = false;
    static long lastsec = 0, lastusec = 0;
    struct variable *v;
    long   realt;
    char   *cpu_elapsed;

    if (!name || eq(name, "totalcputime") || eq(name, "cputime")) {
	int	total, totalu;

#ifdef HAS_BSDRUSAGE
        struct rusage ruse;
        (void) getrusage(RUSAGE_SELF, &ruse);
	total = ruse.ru_utime.tv_sec + ruse.ru_stime.tv_sec;
	totalu = (ruse.ru_utime.tv_usec + ruse.ru_stime.tv_usec) / 1000;
	cpu_elapsed = "CPU";
#else
	struct tms ruse;
	realt = times(&ruse);
	total = (ruse.tms_utime + ruse.tms_stime)/ HZ;
	totalu = (ruse.tms_utime + ruse.tms_utime) * 1000 / HZ;
	cpu_elapsed = "CPU";
#endif

	if (!name || eq(name, "totalcputime")) {
	    total += totalu / 1000;
	    totalu %= 1000;
	    fprintf(cp_out, "Total %s time: %u.%03u seconds.\n",
		cpu_elapsed, total, totalu);
	}

	if (!name || eq(name, "cputime")) {
	    lastusec = totalu - lastusec;
	    lastsec = total - lastsec;
	    while (lastusec < 0) {
		lastusec += 1000;
		lastsec -= 1;
	    }
	    while (lastusec > 1000) {
		lastusec -= 1000;
		lastsec += 1;
	    }

	    fprintf(cp_out, "%s time since last call: %lu.%03lu seconds.\n",
		cpu_elapsed, lastsec, lastusec);

	    lastsec = total;
	    lastusec = totalu;
	}

	yy = true;
    }

    if (!name || eq(name, "space")) {
	long usage = 0, limit = 0;
        struct rlimit rld;
        char *hi;

        getrlimit(RLIMIT_DATA, &rld);
	limit = rld.rlim_cur - (enddata - startdata);
	hi = sbrk(0);
	usage = (long) (hi - enddata);
        fprintf(cp_out, "Current dynamic memory usage = %lu,\n", usage);
        fprintf(cp_out, "Dynamic memory limit = %lu.\n", limit);
        yy = true;
    }

    if (!name || eq(name, "faults")) {
#ifdef HAS_BSDRUSAGE
        struct rusage ruse;

        (void) getrusage(RUSAGE_SELF, &ruse);
        fprintf(cp_out, "%lu page faults, %lu vol + %lu invol = %lu context switches.\n",
                ruse.ru_majflt, ruse.ru_nvcsw, ruse.ru_nivcsw,
                ruse.ru_nvcsw + ruse.ru_nivcsw);
        yy = true;
#endif
    }

    /* Now get all the spice resource stuff. */
    if (ft_curckt && ft_curckt->ci_ckt) {
	wordlist *wl;
	if (name && eq(name, "task"))
	    v = if_getstat(ft_curckt->ci_ckt, NULL);
	else
	    v = if_getstat(ft_curckt->ci_ckt, name);
	if (v) {
	    if (!name) putc('\n', cp_out);
	    while (v) {
		fprintf(cp_out, "%s = ", v->va_name);
		wl = cp_varwl(v);
		wl_print(wl, cp_out); putc('\n', cp_out);
		wl_free(wl);
		if (name) break;
		v = v->va_next;
	    }
	    yy = true;
	}
    }

    if (!yy) {
        fprintf(cp_err, "Note: no resource usage information for '%s',\n", name);
        fprintf(cp_err, "\tor no active circuit available\n");
    }
}

static void * baseaddr()
{
    return 0;
}
