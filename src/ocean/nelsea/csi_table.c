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

/*
 * structure to contain in the CSI (Circuit-String-Instance) hash-table the
 * info of one instance:
 */
typedef struct _CSIBUCKET
{
    CIRCUITPTR circuit;	  /* pointer to the parent of this instance  */
    STRING     name;	  /* the name of the instance */
    CIRINSTPTR instance;  /* pointer to the instance itself */
    struct _CSIBUCKET *next;
}
CSIBUCKET, *CSIBUCKETPTR;

#define NewCsibucket(p) ((p) = (CSIBUCKETPTR)mnew (sizeof(CSIBUCKET)))
#define FreeCsibucket(p) mfree ((char **)(p), sizeof(CSIBUCKET))

/*
 * size of the CSI hash table, should be a prime.
 */
#define CSI_TABLE_SIZ 25013

/*
 * the CSI hash table, initialized to NULL by the compiler!
 */
static CSIBUCKETPTR csi_table[CSI_TABLE_SIZ];
static long csi_table_usage = 0; /* number of buckets currently in use */

/* Compute which entry in the hash table must be used for a T-I pair:
 */
static int csi_entry (CIRCUITPTR circuit, STRING name)
{
    /* This is probably too simple...
     * We need a function with a really good uniform distribution!
     */
    unsigned long hashvalue = (long)circuit + ((long)name << 18) + (long)name;
    return hashvalue % CSI_TABLE_SIZ;
}

/* Add a C-S-I combination to the csi hash table, using the C-S pair as the
 * lookup key. Do not check if C-S-I is already in the csi_table:
 */
void csi_insert (CIRCUITPTR circuit, STRING name, CIRINSTPTR instance)
{
    CSIBUCKETPTR bucket;
    int entry = csi_entry (circuit, name);

    /* create a new bucket and fill it in: */
    NewCsibucket (bucket);
    bucket->circuit = circuit;
    bucket->name = name;
    bucket->instance = instance;

    /* link it in front of the bucket list of the appropriate entry: */
    bucket->next = csi_table[entry];
    csi_table[entry] = bucket;
  ++csi_table_usage;
}

CIRINSTPTR csi_lookup (CIRCUITPTR circuit, STRING name)
{
    CSIBUCKETPTR bucket;
    int entry = csi_entry (circuit, name);

    for (bucket = csi_table[entry]; bucket; bucket = bucket->next)
	if (bucket->circuit == circuit && bucket->name == name) return bucket->instance;
    return NULL;
}

/* Clean up the CSI hashtable if it is filled more than n percent:
 */
void csi_cleanup (int n)
{
    if (n < 0 || n > 100) error (FATAL_ERROR, "csi_cleanup: arg must be in range 0..100");

    if ((double)csi_table_usage / CSI_TABLE_SIZ >= (double)n / 100) {
	int entry;
	CSIBUCKETPTR bucket, nextbucket;

	for (entry = 0; entry < CSI_TABLE_SIZ; ++entry)
	    for (bucket = csi_table[entry]; bucket; bucket = nextbucket) {
		nextbucket = bucket->next;
		FreeCsibucket (bucket);
	    }
    }
}

#define STAT_ARRAY_LENGTH 10

/* Print statistics about the distribution of the hashtable usage.
 * Necessary to tune the hash function csi_entry().
 */
void csi_statistics ()
{
    long entry, length, maxlength = 0, number_of_lists = 0;
    float relative_usage, average_length;
    CSIBUCKETPTR bucket;
/*
 * the number stat_array[length] says how many entries in the csi_table contain
 * a buckets list of this length. First, initialize the stat_array to 0:
 */
    long stat_array[STAT_ARRAY_LENGTH];

    for (length = 0; length < STAT_ARRAY_LENGTH; ++length)
	stat_array[length] = 0;

    for (entry = 0; entry < CSI_TABLE_SIZ; ++entry) {
	length = 0;
	for (bucket = csi_table[entry]; bucket; bucket = bucket->next) ++length;

	if (length > maxlength) maxlength = length; /* keep track of longest list */
	if (length >= STAT_ARRAY_LENGTH) length = STAT_ARRAY_LENGTH - 1;
	++stat_array[length];
    }

    for (length = 1; length < STAT_ARRAY_LENGTH; ++length)
	if (stat_array[length] > 0) number_of_lists += stat_array[length];

    relative_usage = (float)((double)csi_table_usage) / CSI_TABLE_SIZ;
    average_length = (float)((double)csi_table_usage) / number_of_lists;

    printf ("\n---------------------------------------------------------------------\n");
    printf ("       S T A T I S T I C S    O F   T H E    C S I    T A B L E\n\n");
    printf ("table size   = %5d entries\n", CSI_TABLE_SIZ);
    printf ("table usage  = %5ld buckets = %.1f buckets/entry\n", csi_table_usage, (float)relative_usage);
    printf ("maximum list = %5ld buckets\n", maxlength);
    printf ("avarage list = %5.1f buckets\n", (float)average_length);
    printf ("---------------------------------------------------------------------\n");

    for (length = 0; length < STAT_ARRAY_LENGTH; ++length) printf (" %5ld", length);
    printf ("\n");
    for (length = 0; length < STAT_ARRAY_LENGTH; ++length) printf (" %5ld", stat_array[length]);
    printf ("\n---------------------------------------------------------------------\n");
    fflush (stdout);
}
