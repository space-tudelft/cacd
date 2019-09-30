/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
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

#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/prototypes.h"

/* structure to contain in the TIN (Terminal-Instance-Net) hash-table the info
 * of one terminal:
 */
typedef struct _TINBUCKET
{
    CIRPORTPTR term;
    CIRINSTPTR inst;
    NETPTR     net;
    struct _TINBUCKET *next;
}
TINBUCKET, *TINBUCKETPTR;

#define NewTinbucket(p) ((p) = (TINBUCKETPTR)mnew (sizeof(TINBUCKET)))
#define FreeTinbucket(p) mfree ((char **)(p), sizeof(TINBUCKET))

/*
 * size of the TIN hash table, should be a prime.
 */
#define TIN_TABLE_SIZ 25013

/*
 * the TIN hash table, initialized to NULL by the compiler!
 */
static TINBUCKETPTR tin_table[TIN_TABLE_SIZ];
static long tin_table_usage = 0; /* number of buckets currently in use */

/* Compute which entry in the hash table must be used for a T-I pair:
 */
static int tin_entry (CIRPORTPTR term, CIRINSTPTR inst)
{
    /* This is probably too simple...
     * We need a function with a really good uniform distribution!
     */
    unsigned long hashvalue = (long)inst + ((long)term << 18) + (long)term;
    return hashvalue % TIN_TABLE_SIZ;
}

/* Add a T-I-N combination to the tin hash table, using the T-I pair as the
 * lookup key. Do not check if T-I-N is already in the tin_table:
 */
void tin_insert (CIRPORTPTR term, CIRINSTPTR inst, NETPTR net)
{
    TINBUCKETPTR bucket;
    int entry = tin_entry (term, inst);

    /* create a new bucket and fill it in: */
    NewTinbucket (bucket);
    bucket->term = term;
    bucket->inst = inst;
    bucket->net  = net;

    /* link it in front of the bucket list of the appropriate entry: */
    bucket->next = tin_table[entry];
    tin_table[entry] = bucket;
  ++tin_table_usage;
}

NETPTR tin_lookup (CIRPORTPTR term, CIRINSTPTR inst)
{
    TINBUCKETPTR bucket;
    int entry = tin_entry (term, inst);

    for (bucket = tin_table[entry]; bucket; bucket = bucket->next)
	if (bucket->inst == inst && bucket->term == term) return bucket->net;
    return NULL;
}

/* Clean up the TIN hashtable if it is filled more than n percent:
 */
void tin_cleanup (int n)
{
    if (n < 0 || n > 100) error (FATAL_ERROR, "tin_cleanup: arg must be in range 0..100");

    if ((double)tin_table_usage / TIN_TABLE_SIZ >= (double)n / 100) {
	int entry;
	TINBUCKETPTR bucket, nextbucket;

	for (entry = 0; entry < TIN_TABLE_SIZ; ++entry)
	    for (bucket = tin_table[entry]; bucket; bucket = nextbucket) {
		nextbucket = bucket->next;
		FreeTinbucket (bucket);
	    }
    }
}

#define STAT_ARRAY_LENGTH 10

/* Print statistics about the distribution of the hashtable usage.
 * Necessary to tune the hash function tin_entry()
 */
void tin_statistics ()
{
    long entry, length, maxlength = 0, number_of_lists = 0;
    float relative_usage, average_length;
    TINBUCKETPTR bucket;
/*
 * the number stat_array[length] says how many entries in the tin_table contain
 * a buckets list of this length. First, initialize the stat_array to 0:
 */
    long stat_array[STAT_ARRAY_LENGTH];

    for (length = 0; length < STAT_ARRAY_LENGTH; ++length)
	stat_array[length] = 0;

    for (entry = 0; entry < TIN_TABLE_SIZ; ++entry) {
	length = 0;
	for (bucket = tin_table[entry]; bucket; bucket = bucket->next) ++length;

	if (length > maxlength) maxlength = length; /* keep track of longest list */
	if (length >= STAT_ARRAY_LENGTH) length = STAT_ARRAY_LENGTH - 1;
	++stat_array[length];
    }

    for (length = 1; length < STAT_ARRAY_LENGTH; ++length)
	if (stat_array[length] > 0) number_of_lists += stat_array[length];

    relative_usage = (float)((double)tin_table_usage) / TIN_TABLE_SIZ;
    average_length = (float)((double)tin_table_usage) / number_of_lists;

    printf ("\n---------------------------------------------------------------------\n");
    printf ("       S T A T I S T I C S    O F   T H E    T I N    T A B L E\n\n");
    printf ("table size   = %5d entries\n", TIN_TABLE_SIZ);
    printf ("table usage  = %5ld buckets = %.1f buckets/entry\n", tin_table_usage, (float)relative_usage);
    printf ("maximum list = %5ld buckets\n", maxlength);
    printf ("avarage list = %5.1f buckets\n", (float)average_length);
    printf ("---------------------------------------------------------------------\n");

    for (length = 0; length < STAT_ARRAY_LENGTH; ++length) printf (" %5ld", length);
    printf ("\n");
    for (length = 0; length < STAT_ARRAY_LENGTH; ++length) printf (" %5ld", stat_array[length]);
    printf ("\n---------------------------------------------------------------------\n");
    fflush (stdout);
}
