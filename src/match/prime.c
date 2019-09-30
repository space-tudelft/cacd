static char *rcsid = "$Id: prime.c,v 1.1 2018/04/30 12:17:43 simon Exp $";
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
 * This module contains a number of functions
 * to generate prime numbers.
 * These numbers are used to code the permutability
 * of the pins of a device.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

typedef struct p_list {
    struct p_list  *next;
    unsigned long prime;
    unsigned long square;
} p_list;

Private p_list * prime_list = NULL;/* pointer to head of list  */
Private p_list * prime_tail = NULL;/* pointer to tail of list  */
Private p_list * prime_pntr = NULL;/* pointer to current prime */

/*
 * Sets the pointer of the prime generator to the first prime.
 */
Public void rewind_gen ()
{
    prime_pntr = prime_list;
}

/*
 * Generates the next prime.
 */
Public long gen_prime ()
{
    unsigned long num;
    unsigned long prime;
    boolean ThiS_is_prime;
    p_list * ThiS;
    p_list * pnew;

 /* only the first time */

    if (prime_list == NULL) {
	Malloc (prime_list, 1, p_list);
	prime_list -> prime = 2;
	prime_list -> square = 4;
	prime_list -> next = NULL;
	prime_tail = prime_list;
	prime_pntr = prime_list;
    }

    if (prime_pntr == NULL) {

	for (num = prime_tail -> prime + 1;; num++) {

	    ThiS_is_prime = True;
	    for (ThiS = prime_list; ThiS && ThiS -> square <= num; ThiS = ThiS -> next) {
		if (num % ThiS -> prime == 0) {
		    ThiS_is_prime = False;
		    break;
		}
	    }
	    if (ThiS_is_prime) {
		Malloc (pnew, 1, p_list);
		pnew -> prime = num;
		pnew -> square = num * num;
		pnew -> next = NULL;
		prime_tail -> next = pnew;
		prime_tail = pnew;
		prime_pntr = pnew;
		break;
	    }
	}
    }

    prime = prime_pntr -> prime;
    prime_pntr = prime_pntr -> next;
 /*
  fprintf (stderr, "generated prime is: %ld\n", prime);
  */
    return (prime);
}
