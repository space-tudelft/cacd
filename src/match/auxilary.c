static char *rcsid = "$Id: auxilary.c,v 1.1 2018/04/30 12:17:23 simon Exp $";
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
 * Contains a number of special general purpose functions.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

Public long  n_netw_alloc;
Public long  n_objt_alloc;
Public long  n_list_alloc;
Public long  n_link_alloc;
Public long  n_part_alloc;
Public long  n_blck_alloc;
Public long  n_buck_alloc;
Public long  n_strn_alloc, n_strn_mem;
Public long  n_arr_alloc , n_arr_mem;
Public long  n_hash_alloc, n_hash_mem;

/*
 * Dynamically allocates space for the string so that it
 * will be resident in memory among function invocations.
 * The string is copied into the newmem space and a pointer
 * to it is returned.
 * To be used in conjunction with deletemem().
 */
Public string newmem (string str)
{
    string n_str;
    long   str_l;

    str_l = strlen (str) + 1;
    Malloc (n_str, str_l, char);
    n_strn_alloc++;
    n_strn_mem += str_l;
    strcpy (n_str, str);

    return (n_str);
}

/*
 * Removes the string 'str' from dynamic memory.
 * To be used in conjunction with newmem().
 */
Public void deletemem (string str)
{
    if (str) {
	n_strn_alloc--;
	n_strn_mem -= strlen (str) + 1;
	Free (str);
    }
}

/*
 * Adds the index between brackets to the specified name.
 * If the name already contains indices, the
 * newmem index is inserted at the head.
 * A pointer to the newmem string is returned.
 */
Public string add_index (string name, long index)
{
    char work[DM_MAXNAME+40];
    int i = 0;

    Assert (name);

    while (name[i] && name[i] != '[') { work[i] = name[i]; ++i; }

    if (name[i] == '[') {
	sprintf (work+i, "[%ld,%s", index, name + i + 1);
    }
    else {
	sprintf (work+i, "[%ld]", index);
    }

    return (mk_symbol (work));
}

/*
 * Allocates the specified amount of
 * memory and returns a pointer to it.
 * A number of statistical parameters is maintained.
 */
Private long times_inv = 0;	/* Invokations of malloc */
Private long tot_mem = 0;	/* Total allocated memory  */
Private long max_mem = 0;	/* Maximum allocated memory  */

Public void *s_malloc (unsigned long amount)
{
    long *new_mem;

    if ((new_mem = (long *)malloc (amount + sizeof (long)))) {

	tot_mem += amount;
	max_mem += amount;
	times_inv++;
	*new_mem = amount;
	return (new_mem + sizeof (long));
    }
    return (NULL);
}

/*
 * Frees the memory pointed to by 'ThiS'.
 * A number of statistical parameters is maintained.
 */
Public void s_free (void *ThiS)
{
    if (ThiS) {
	ThiS -= sizeof (long);
	tot_mem -= *((long *) ThiS);
	free (ThiS);
    }
}

/*
 * Memory allocation error report.
 */
Public void m_err (int amount)
{
    fprintf (stderr, "malloc: error, cannot allocate %d bytes.\n\n", amount);
    /* astats (); */
    my_exit (1);
}

/*
 * Prints some statistics about the memory allocation
 * on the standard error output.
 */
Public void astats ()
{
    long     amount;
    long     objects = 0;
    long     total = 0;

    fprintf (stderr, "MEMORY ALLOCATION STATISTICS\n");
    fprintf (stderr, "----------------------------\n\n");
#ifdef DEBUG
    fprintf (stderr, "Maximum amount of memory allocated:          %8ld bytes\n", max_mem);
    fprintf (stderr, "Total amount of memory allocated:            %8ld bytes\n", tot_mem);
    fprintf (stderr, "Total number of invocations to malloc:       %8ld times\n", times_inv);
    fprintf (stderr, "\n");
#endif
    amount = n_netw_alloc * sizeof (network);
    fprintf (stderr, "Total allocated netws:  %5ld * (%3u bytes) = %7ld bytes\n",
	    n_netw_alloc, (unsigned int)sizeof (network), amount);
    total += amount;
    objects += n_netw_alloc;
    amount = n_objt_alloc * sizeof (object);
    fprintf (stderr, "Total allocated objts:  %5ld * (%3u bytes) = %7ld bytes\n",
	    n_objt_alloc, (unsigned int)sizeof (object), amount);
    total += amount;
    objects += n_objt_alloc;
    amount = n_link_alloc * sizeof (link_type);
    fprintf (stderr, "Total allocated links:  %5ld * (%3u bytes) = %7ld bytes\n",
	    n_link_alloc, (unsigned int)sizeof (link_type), amount);
    total += amount;
    objects += n_link_alloc;
    amount = n_part_alloc * sizeof (partition);
    fprintf (stderr, "Total allocated parts:  %5ld * (%3u bytes) = %7ld bytes\n",
	    n_part_alloc, (unsigned int)sizeof (partition), amount);
    total += amount;
    objects += n_part_alloc;

    amount = n_blck_alloc * sizeof (block);
    fprintf (stderr, "Total allocated blocks: %5ld * (%3u bytes) = %7ld bytes\n",
	    n_blck_alloc, (unsigned int)sizeof (block), amount);
    total += amount;
    objects += n_blck_alloc;

    fprintf (stderr, "Total allocated arrays: %5ld * (... bytes) = %7ld bytes\n",
	    n_arr_alloc, n_arr_mem);
    total += n_arr_mem;
    objects += n_arr_alloc;

    amount = n_buck_alloc * sizeof (bucket);
    fprintf (stderr, "Total allocated bucks:  %5ld * (%3u bytes) = %7ld bytes\n",
	    n_buck_alloc, (unsigned int)sizeof (bucket), amount);
    total += amount;
    objects += n_buck_alloc;

    amount = n_list_alloc * sizeof (list);
    fprintf (stderr, "Total allocated lists:  %5ld * (%3u bytes) = %7ld bytes\n",
	    n_list_alloc, (unsigned int)sizeof (list), amount);
    total += amount;
    objects += n_list_alloc;

    fprintf (stderr, "Total allocated hashs:  %5ld * (... bytes) = %7ld bytes\n",
	    n_hash_alloc, n_hash_mem);
    total += n_hash_mem;
    objects += n_hash_alloc;

    fprintf (stderr, "Total allocated strings %5ld * (... bytes) = %7ld bytes\n",
	    n_strn_alloc, n_strn_mem);
    total += n_strn_mem;
    objects += n_strn_alloc;

    fprintf (stderr, "%31s %29s\n", "----- +", "------------- +");
    fprintf (stderr, "Total: %22ld %23ld bytes\n", objects, total);
    fprintf (stderr, "\n");
}
