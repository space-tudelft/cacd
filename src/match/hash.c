static char *rcsid = "$Id: hash.c,v 1.1 2018/04/30 12:17:29 simon Exp $";
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
 * This module contains a number of functions which
 * maintain and access a hashing table.
 * Such a hashing table is an index structure which
 * provides a fast access to a collection of objects.
 * The key on which the objects are indexed is a
 * string (The name of the object).
 */
#include "src/match/head.h"
#include "src/match/proto.h"

Private int hash_f (hash *hash_pnt, string key);

Import long n_buck_alloc;
Import long n_hash_alloc;
Import long n_hash_mem;

/*
 * Creates a hash index structure.
 * The hash_table is of the specified length.
 * A pointer to the hash structure is returned.
 */
Public hash *mk_hash (int hash_size)
{
    hash    *hash_pnt;
    bucket **hash_table;
    int i;

    Assert (hash_size > 0);

    Malloc (hash_pnt, 1, hash);
    Malloc (hash_table, hash_size, bucket *);
    n_hash_alloc++;
    n_hash_mem += sizeof (hash);
    n_hash_mem += hash_size * sizeof (bucket *);

    hash_pnt -> bucks = 0;
    hash_pnt -> size  = hash_size;
    hash_pnt -> table = hash_table;

    for (i = 0; i < hash_size; i++) hash_table[i] = NULL;

    return (hash_pnt);
}

/*
 * Removes a hash index structure.
 */
Public void rm_hash (hash *hash_pnt)
{
    bucket *ThiS, *next;
    int i;

    Assert (hash_pnt);

    if (hash_pnt -> bucks > 0)
    for (i = 0; i < hash_pnt -> size; i++) {
	for (ThiS = hash_pnt -> table[i]; ThiS; ThiS = next) {
	    next = ThiS -> next;
	    (void) rm_a_bucket (ThiS);
	    if (--hash_pnt -> bucks == 0) goto ret;
	}
    }
ret:
    n_hash_mem -= sizeof (hash);
    n_hash_mem -= hash_pnt -> size * sizeof (bucket *);
    n_hash_alloc--;
    Free (hash_pnt -> table);
    Free (hash_pnt);
}

/*
 * Links the object pointed to by 'data' under
 * the key 'key' in the hash structure.
 */
Public void h_link (hash *hash_pnt, string key, void *data)
{
    bucket *ThiS;
    int i;

    Assert (key && data);

    i = hash_f (hash_pnt, key);
    ThiS = mk_a_bucket ();
    ThiS -> data = data;
    ThiS -> key  = key;
    ThiS -> next = hash_pnt -> table[i];
    hash_pnt -> table[i] = ThiS;
    hash_pnt -> bucks++;
    Assert (hash_pnt -> bucks > 0);
}

/*
 * Unlinks the object stored under the 'key' from the hash
 * structure and returns a pointer to the unlinked object.
 * NULL is returned when 'key' is not present.
 */
Public void *h_unlink (hash *hash_pnt, string key, int rm_key)
{
    bucket *p1, *p2;
    int i;

    Assert (key);
    if (!hash_pnt -> bucks) goto ret;

    i = hash_f (hash_pnt, key);

    p1 = hash_pnt -> table[i];
    p2 = NULL;

    /* scan the linked list for 'key' */
    while (p1) {
	if (strcmp (p1 -> key, key) == 0) {
	    if (rm_key) rm_symbol (p1 -> key);
	    if (p2) p2 -> next = p1 -> next;
	    else hash_pnt -> table[i] = p1 -> next;
	    hash_pnt -> bucks--;
	    return (rm_a_bucket (p1));
	}
	p2 = p1;
	p1 = p2 -> next;
    }
ret:
    if (!rm_key) fprintf (stderr, "WARNING: Could not unlink %s\n", key);
    return (NULL);
}

/*
 * Returns a pointer to the object stored
 * under the 'key' in the hash structure.
 * When 'key' is not present, NULL is returned.
 */
Public void *h_get (hash *hash_pnt, string key)
{
    bucket *ThiS;

    ThiS = hash_pnt -> table[hash_f (hash_pnt, key)];
    while (ThiS) {
	if (strcmp (ThiS -> key, key) == 0) return (ThiS -> data);
	ThiS = ThiS -> next;
    }
    return (NULL);
}

/*
 * Prints the hash structure on the specified stream.
 * Only used for debugging purposes.
 */
Public void p_hash (FILE *fp, hash *hash_pnt)
{
    bucket *ThiS;
    int i;

    if (!hash_pnt) return;
    fprintf (fp, "INDEX:\t| %-12.12s | DATA\n", "KEY");
    fprintf (fp, "--------+--------------+---------\n");
    for (i = 0; i < hash_pnt -> size; i++) {
	if (!(ThiS = hash_pnt -> table[i])) continue;
	fprintf (fp, "%5d:", i);
	for (; ThiS; ThiS = ThiS -> next)
	    fprintf (fp, "\t| %-12.12s | %p\n", ThiS -> key, ThiS -> data);
    }
    fprintf (fp, "--------+--------------+---------\n\n");
}

/*
 * Basic hash function.
 * Calculates a hash value for each key based on
 * the exor sum of the ascii values of the characters of
 * the string 'key'.
 */
Private int hash_f (hash *hash_pnt, string key)
{
	int hash_value = 0;

	while (*key) {
		hash_value ^= *key++;
		if (!*key) break;
		hash_value ^= (*key++ << 8);
		if (!*key) break;
		hash_value ^= (*key++ << 16);
		if (!*key) break;
		hash_value ^= (*key++ << 24);
	}
    /*
    if (hash_pnt -> size == 0) fprintf (stderr, "-- hash_pnt -> size == 0\n");
    */
    return (hash_value % hash_pnt -> size);
}

/*
 * Creates a bucket for queue/stack/hash data structure.
 * A pointer to the bucket is returned.
 */
Private bucket * empty_buckets = NULL;

Public bucket * mk_a_bucket ()
{
    bucket * ThiS_bucket;

    if (empty_buckets) {
	ThiS_bucket = empty_buckets;
	empty_buckets = ThiS_bucket -> next;
    }
    else {
	Malloc (ThiS_bucket, 1, bucket);
    }

    ThiS_bucket -> key  = NULL;
    ThiS_bucket -> data = NULL;
    ThiS_bucket -> next = NULL;
    ThiS_bucket -> prev = NULL;

    n_buck_alloc++;

    return (ThiS_bucket);
}

/*
 * Removes the bucket.
 * A pointer to the bucket data is returned.
 */
Public void *rm_a_bucket (bucket *ThiS_bucket)
{
    ThiS_bucket -> next = empty_buckets;
    empty_buckets = ThiS_bucket;

    n_buck_alloc--;

    return (ThiS_bucket -> data);
}
