/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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
 * This file provides functions to store string values and an associated void
 * pointer in a hash table. After this, you can lookup these STV (string,
 * void*) pairs.  You can also erase the hashtable efficiently.
 *
 * WARNING: ===>>> the string must be canonic <<===
 */

#include <stdio.h>
#include <stdlib.h>
#include "src/ocean/libseadif/sea_decl.h"

/*
 * structure to contain in the STV hash-table the info of one string:
 */
typedef struct _STVBUCKET
{
   STRING string;
   void *pointer;
   struct _STVBUCKET *next;
}
STVBUCKET, *STVBUCKETPTR;

#define NewStvbucket(p) ((p) = (STVBUCKETPTR)mnew (sizeof(STVBUCKET)))
#define FreeStvbucket(p) mfree ((char **)(p), sizeof(STVBUCKET))

/*
 * the empty hashtable is an array of these structures:
 */
typedef struct _STVENTRY
{
   STVBUCKETPTR bucketlist;
   struct _STVENTRY *next;
}
STVENTRY, *STVENTRYPTR;

/*
 * size of the STV hash table, should be a prime.
 */
#define STV_TABLE_SIZ 25013

/*
 * the STV hash table:
 */
static STVENTRYPTR stv_table;
static STVENTRYPTR stv_used_slots; /* list of used slots in the table */
static long stv_table_usage; /* number of buckets currently in use */

/* Create the STV table, initialize it to NULL:
 */
static void init_stv_table ()
{
   int i; unsigned alloc_siz = STV_TABLE_SIZ * sizeof(STVENTRY);

   stv_table = (STVENTRYPTR)malloc (alloc_siz);
   if (!stv_table) sdfreport (Fatal, "cannot allocate memory (%d bytes) for STV hash table\n", alloc_siz);

   for (i = 0; i < STV_TABLE_SIZ; ++i) {
      stv_table[i].bucketlist = NULL;
      stv_table[i].next = NULL;
   }
   stv_used_slots = NULL;
   stv_table_usage = 0;
}

/* Compute which entry in the hash table must be used for a S-V pair:
 */
static int stv_entry (STRING string)
{
   /* This is probably too simple...
    * We need a function with a really good uniform distribution!
    */
   unsigned long hashvalue = ((long)string << 18) + (long)string;
   return hashvalue % STV_TABLE_SIZ;
}

/* Add an S-V combination to the stv hash table, using the String as
 * the lookup key. Do not check if S-V is already in the stv_table:
 */
void sdfstv_insert (STRING string, void *pointer)
{
   STVBUCKETPTR bucket;
   int entry = stv_entry (string);

   if (!stv_table) init_stv_table (); /* first call initializes the hash table */

   /* link this slot in the list of used slots, if not already done: */
   if (!stv_table[entry].bucketlist) {
      stv_table[entry].next = stv_used_slots;
      stv_used_slots = &(stv_table[entry]);
   }

   /* create a new bucket and fill it in: */
   NewStvbucket (bucket);
   bucket->string = string;
   bucket->pointer = pointer;

   /* link it in front of the bucket list: */
   bucket->next = stv_table[entry].bucketlist;
   stv_table[entry].bucketlist = bucket;
   /* account for this bucket: */
   stv_table_usage += 1;
}

/* Lookup the STRING in the STV hash table and return the associated
 * void pointer:
 */
void *sdfstv_lookup (STRING string)
{
   STVBUCKETPTR bucket;
   int entry = stv_entry (string);

   for (bucket = stv_table[entry].bucketlist; bucket; bucket = bucket->next)
      if (bucket->string == string) return bucket->pointer;
   return NULL;
}

/* Clean up the STV hashtable:
 */
void sdfstv_cleanup ()
{
   STVENTRYPTR used_slot, next_used_slot;

   for (used_slot = stv_used_slots; used_slot; used_slot = next_used_slot)
   {
      STVBUCKETPTR bucket, nextbucket;
      for (bucket = used_slot->bucketlist; bucket; bucket = nextbucket)
      {
	 nextbucket = bucket->next;
	 FreeStvbucket (bucket);
      }
      used_slot->bucketlist = NULL;
      next_used_slot = used_slot->next;
      used_slot->next = NULL;
   }
   stv_table_usage = 0;
   stv_used_slots = NULL;
}

#define STAT_ARRAY_LENGTH 10

/* Print statistics about the distribution of the hashtable usage.
 * Necessary to tune the hash function stv_entry().
 */
void sdfstv_statistics ()
{
   long entry, length, maxlength = 0, number_of_lists = 0;
   float relative_usage, average_length;
   STVBUCKETPTR bucket;
   /*
    * the number stat_array[length] says how many entries in the stv_table contain
    * a buckets list of this length. First, initialize the stat_array to 0:
    */
   long stat_array[STAT_ARRAY_LENGTH];

   for (length = 0; length < STAT_ARRAY_LENGTH; ++length)
      stat_array[length] = 0;

   for (entry = 0; entry < STV_TABLE_SIZ; ++entry) {
      length = 0;
      for (bucket = stv_table[entry].bucketlist; bucket; bucket = bucket->next) ++length;

      if (length > maxlength) maxlength = length; /* keep track of longest list */
      if (length >= STAT_ARRAY_LENGTH) length = STAT_ARRAY_LENGTH - 1;
      stat_array[length] += 1;
   }

   for (length = 1; length < STAT_ARRAY_LENGTH; ++length)
      if (stat_array[length] > 0) number_of_lists += stat_array[length];

   relative_usage = (float)((double)stv_table_usage)/STV_TABLE_SIZ;
   average_length = (float)((double)stv_table_usage)/number_of_lists;

   printf ("\n---------------------------------------------------------------------\n");
   printf ("       S T A T I S T I C S    O F   T H E    S T V    T A B L E\n\n");
   printf ("table size   = %5d entries\n", STV_TABLE_SIZ);
   printf ("table usage  = %5ld buckets = %.1f buckets/entry\n", stv_table_usage, (float)relative_usage);
   printf ("maximum list = %5ld buckets\n", maxlength);
   printf ("avarage list = %5.1f buckets\n", (float)average_length);
   printf ("---------------------------------------------------------------------\n");

   for (length = 0; length < STAT_ARRAY_LENGTH; ++length) printf (" %5ld", length);
   printf ("\n");
   for (length = 0; length < STAT_ARRAY_LENGTH; ++length) printf (" %5ld", stat_array[length]);
   printf ("\n---------------------------------------------------------------------\n");
   fflush (stdout);
}
