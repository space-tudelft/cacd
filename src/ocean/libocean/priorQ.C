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

#include <iostream>
using namespace std;
#include <stdlib.h>		// abort()
#include "src/ocean/libocean/priorQ.h"

// construction of the priority queue:
priorQ::priorQ (int maxSize)
{
   if (maxSize < 2) priorQerror ("construction out of range");
   maxHeapSize = maxSize;
   heap = new priorQelmt* [maxHeapSize+1]; // allocate space for the heap
   heapSize = 0;			  // initially, the heap is empty
}

// destruction of the priority queue:
priorQ::~priorQ ()
{
   delete heap;
}

// heapify the priority queue. This lets the elmt heap[i] float down in the
// heap so that the subtree rooted at index i becomes a heap:
void priorQ::heapify (int i)
{
   if (heapSize < 1) return;	// no heap to heapify ...
   if (i < 1 || i > heapSize) priorQerror ("heapify out of range");
   int l = left(i);
   int r = right(i);
   // set "highest" to one of (i, l, r) that has the highest priority:
   int highest = i;
   if (l <= heapSize && heap[l]->priority() > heap[i]->priority())
      highest = l;
   if (r <= heapSize && heap[r]->priority() > heap[highest]->priority())
      highest = r;
   if (highest != i)
   {
      exchange (i, highest);
      heapify (highest);
   }
}

// exchange two entries in the heap:
void priorQ::exchange (int i, int j)
{
   if (i < 1 || j < 1 || i > heapSize || j > heapSize) priorQerror ("exchange out of range");
   priorQelmt *tmp = heap[i];
   heap[i] = heap[j];
   heap[j] = tmp;
}

// type a message and exit:
void priorQ::priorQerror (const char *s)
{
   cout << flush;
   cerr << "\nInternal priority queue error: " << s << endl;
   abort();
}

// extract the element with the highest priority from the queue:
priorQelmt *priorQ::extractFirst ()
{
   if (heapSize < 1) priorQerror ("extract from empty queue");
   priorQelmt *first = heap[1];    // get the elmt with highest priority
   heap[1] = heap[heapSize--];	   // decrease the heap size by 1
   heapify(1);			   // maintain the heap property
   return first;
}

// return (not extract) the elmt with the highest priorority:
const priorQelmt *priorQ::first ()
{
   if (heapSize < 1) priorQerror ("access of empty queue");
   return heap[1];
}

// insert a new element into the priority queue; maintain the heap property:
priorQ& priorQ::insert (priorQelmt& elmt)
{
   int key = elmt.priority();
   if (++heapSize > maxHeapSize) increaseHeapSize();
   int i = heapSize;
   while (i > 1 && heap[parent(i)]->priority() < key) {
      heap[i] = heap[parent(i)];
      i = parent(i);
   }
   heap[i] = &elmt;
   return *this;
}

// return the number of elements in the priority queue:
const int priorQ::qSize ()
{
   return heapSize;
}

// replace the heap by a bigger heap:
void priorQ::increaseHeapSize ()
{
   int previousSize = maxHeapSize;
   maxHeapSize *= 4;		// make the new heap 4 times as big
   priorQelmt **newHeap = new priorQelmt* [maxHeapSize+1];
   // copy the old heap to the new heap:
   int thesize = previousSize; // thesize = minimum(previousSize,heapSize)
   if (heapSize < thesize) thesize = heapSize;
   for (int i = 1; i <= thesize; ++i) newHeap[i] = heap[i];
   delete heap;			// delete the old heap
   heap = newHeap;		// install the new heap
}

// make the priority queue empty:
void priorQ::cleanup (priorQcleanupMode mode)
{
   if (mode == DeletePriorQelmts)
      for (int i = 1; i <= heapSize; ++i) delete heap[i];
   heapSize = 0;
}
