/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
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

#include <string.h>
#include <iostream>
using namespace std;
#include <stdlib.h>		// abort()
#include "src/ocean/libocean/sdfset.h"

sdfset::sdfset (sdfsetOrdering ord, char *thename, int maintainBackwardReference)
{
   firstelmt = lastelmt = NULL;
   ordering = ord;
   name = NULL;
   elmtcount = 0;
   this->putName (thename);
   maintainBackRef = maintainBackwardReference;
}

void sdfset::putName (char *thename)
{
   if (name) delete name;	// first delete the previous name

   if (!thename) name = NULL;
   else {
      name = new char[strlen(thename) + 1];
      strcpy (name, thename);
   }
}

sdfset::~sdfset ()
{
   // Here we clean up the entire set...
}

sdfset *sdfset::add (sdfsetElmt *elmt)
{
   if (elmt)
   {
      if (elmt->nxt || elmt->prv || elmt->myset)
	 sdfsetError ("sdfset::add() -- sdfsetElmt already used in another set...");
      sdfsetElmt *e1 = NULL, *e2 = NULL;
      if (ordering == IncreasingOrderedSet)
      {
	 // find the right place to insert the new elmt
	 int ord = elmt->ordinate();
	 for (e1 = lastelmt; e1; e1 = e1->prv)
	    if (ord >= e1->ordinate())
	       break;
      }
      else if (ordering == DecreasingOrderedSet)
      {
	 // find the right place to insert the new elmt
	 int ord = elmt->ordinate();
	 for (e1 = lastelmt; e1; e1 = e1->prv)
	    if (ord <= e1->ordinate())
	       break;
      }
      else
	 e1 = lastelmt;
      // now link elmt between the sdfsetElmts e1 and e2:
      elmt->prv = e1;
      if (!e1)
      {
	 e2 = firstelmt;
	 firstelmt = elmt;
      }
      else
      {
	 e2 = e1->nxt;
	 e1->nxt = elmt;
      }
      elmt->nxt = e2;
      if (!e2)
	 lastelmt = elmt;
      else
	 e2->prv = elmt;
      elmtcount += 1;		// Keep track of total number of elements
   }
   if (maintainBackRef)
      elmt->myset = this;	// registrate that elmt is now in this set
   postElmtInstallHandler (elmt);
   return this;
}

// remove an element from the set. Do NOT DELETE elmt.
sdfset *sdfset::remove (sdfsetElmt *elmt)
{
   if (maintainBackRef && this != elmt->myset)
      sdfsetError ("attempt to remove sdfsetElmt from wrong set");
   if (!elmt)
      ;
   else if (!elmt->prv)
   {
      // special case, elmt is the first element in the list
      firstelmt = elmt->nxt;
      if (!firstelmt)	// check for only one set element in the list
	 lastelmt = NULL;
      else
	 firstelmt->prv = NULL;
   }
   else if (!elmt->nxt)
   {
      // special case, elmt is last element in the list
      lastelmt = elmt->prv; // prv MUST be != NULL since we tested this above...
      lastelmt->nxt = NULL;
   }
   else
   {
      // general case, elmt is in the middle of the element list
      elmt->prv->nxt = elmt->nxt;
      elmt->nxt->prv = elmt->prv;
   }
   elmt->nxt = elmt->prv = NULL;	// sdfset::add() checks this to see if elmt is free
   elmt->myset = NULL;
   elmtcount -= 1;		// Keep track of total number of elements
   return this;
}

// remove elmt from the set, THEN DELETE IT...:
sdfset *sdfset::del (sdfsetElmt *elmt)
{
   remove (elmt);
   delete elmt;
   return this;
}

// remove all members from the set !!!!! AND THEN DELETE THEM !!!!!
sdfset *sdfset::delAll ()
{
   sdfsetElmt *e;
   while (firstelmt)
   {
      remove (e = firstelmt);
      delete e;
   }
   return this;
}

int sdfset::isEmpty ()
{
   return !this || !firstelmt;
}

// move all elements from set S to this set, leaving S empty...:
sdfset *sdfset::move (sdfset *s)
{
   if (this == s) return this;	// trivial
   if (ordering == NonOrderedSet)
   {
      if (maintainBackRef)
	 // change the backward references from s to this...:
	 for (sdfsetElmt *e = s->firstelmt; e; e = e->nxt)
	    e->myset = this;
      // the rest we can do very quickly...:
      if (!firstelmt)	// this also implies that lastelmt == NULL
	 firstelmt = s->firstelmt;
      else
	 lastelmt->nxt = s->firstelmt;
      if (s->firstelmt) s->firstelmt->prv = lastelmt;
      lastelmt = s->lastelmt;
      s->firstelmt = s->lastelmt = NULL;
      elmtcount += s->elmtcount;
      s->elmtcount = 0;
   }
   else
      // there may exist ways to do it more efficiently, but heck! who cares...
      for (sdfsetElmt *e = s->firstelmt; e; e = e->nxt)
      {
	 s->remove(e);
	 add(e);
      }
   return this;
}

// unite this set and s:
sdfset *sdfset::unite (sdfset *s)
{
   return move(s);		// alias
}

void sdfset::print ()
{
   cout << "SET \"" << (!name ? "<unnamed>" : name) << "\" { ";
   for (sdfsetElmt *elmt = firstelmt; elmt;)
   {
      elmt->print();
      if ((elmt = elmt->nxt)) cout << ", ";
   }
   cout << " }\n" << flush;
}

sdfsetElmt *sdfset::nextElmt (sdfsetElmt *e)
{
   return e->nxt;
}

sdfsetElmt *sdfset::prevElmt (sdfsetElmt *e)
{
   return e->prv;
}

// this checks the internal consistency of the set. For debugging purposes:
void sdfset::ckConsistency ()
{
   if ((!firstelmt && lastelmt) || (firstelmt && !lastelmt))
      sdfsetError ("consistency 1");

   int cnt = 0;
   sdfsetElmt *prev_elmt = NULL, *elmt = firstelmt;
   while (elmt)
   {
      if (prev_elmt != elmt->prv)
	 sdfsetError ("consistency 2");
      if (maintainBackRef && elmt->myset != this)
	 sdfsetError ("consistency 3");
      prev_elmt = elmt;
      elmt = elmt->nxt;
      cnt += 1;
   }
   if (prev_elmt != lastelmt)
      sdfsetError ("consistency 4");
   if (cnt != elmtcount)
      sdfsetError ("consistency 5");
}

void sdfsetElmt::print ()
{
   cout << "<sdfsetElmt>" ;
}

sdfsetNextElmtIterator::sdfsetNextElmtIterator (sdfset *s)
{
   initialize (s);
}

sdfsetNextElmtIterator::sdfsetNextElmtIterator (sdfset *s, int ord) // constructor
{
   filter = TRUE;
   initialize (s, ord);
}

void sdfsetNextElmtIterator::initialize (sdfset *s)
{
   filter = 0; ordinate = 0; theset = s;
   initialize2 ();
}

void sdfsetNextElmtIterator::initialize (sdfset *s, int ord)
{
   filter = TRUE; ordinate = ord; theset = s;
   initialize2 ();
}

void sdfsetNextElmtIterator::initialize2 ()
{
   if (!theset) {
      currentelmt = NULL;
      return;
   }
   currentelmt = theset->firstelmt;
   if (filter)
      while (currentelmt && currentelmt->ordinate() != ordinate)
	 currentelmt = theset->nextElmt(currentelmt);
}

sdfsetElmt *sdfsetNextElmtIterator::operator() () // return next sdfsetElmt in set
{
   sdfsetElmt *thiselmt = currentelmt;
   if (currentelmt) currentelmt = theset->nextElmt (currentelmt);
   if (filter)
      while (currentelmt && currentelmt->ordinate() != ordinate)
	 currentelmt = theset->nextElmt (currentelmt);
   return thiselmt;
}

// if ELM is the next thing to return, then forget about it and advance the
// iterator to the next element. This is a useful method to call when we are
// deleting elements from a set while iterating them at the same time.
void sdfsetNextElmtIterator::skip (sdfsetElmt* elm)
{
   if (currentelmt == elm)
      operator() ();
}

sdfsetPrevElmtIterator::sdfsetPrevElmtIterator (sdfset *s)
{
   initialize (s);
}

sdfsetPrevElmtIterator::sdfsetPrevElmtIterator (sdfset *s, int ord) // constructor
{
   filter = TRUE;
   initialize (s, ord);
}

void sdfsetPrevElmtIterator::initialize (sdfset *s)
{
   filter = 0; ordinate = 0; theset = s;
   initialize2 ();
}

void sdfsetPrevElmtIterator::initialize (sdfset *s, int ord)
{
   filter = TRUE; ordinate = ord; theset = s;
   initialize2 ();
}

void sdfsetPrevElmtIterator::initialize2 ()
{
   if (!theset) {
      currentelmt = NULL;
      return;
   }
   currentelmt = theset->lastelmt;
   if (filter)
      while (currentelmt && currentelmt->ordinate() != ordinate)
	 currentelmt = theset->prevElmt (currentelmt);
}

sdfsetElmt *sdfsetPrevElmtIterator::operator() () // return previous sdfsetElmt in set
{
   sdfsetElmt *thiselmt = currentelmt;
   if (currentelmt) currentelmt = theset->prevElmt (currentelmt);
   if (filter)
      while (currentelmt && currentelmt->ordinate() != ordinate)
	 currentelmt = theset->prevElmt (currentelmt);
   return thiselmt;
}

// if ELM is the next thing to return, then forget about it and advance the
// iterator to the next element. This is a useful method to call when we are
// deleting elements from a set while iterating them at the same time.
void sdfsetPrevElmtIterator::skip (sdfsetElmt* elm)
{
   if (currentelmt == elm) operator() ();
}

void sdfsetError (const char *msg)
{
   cout << flush;
   cerr << "\n/\\/\\/\\/ sdfsetError: " << msg << endl;
   abort();
}
