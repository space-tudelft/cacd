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

// This implements the classes priorQ and priorQelmt that represent a priority
// queue. The priority queue is implemented as a heap. A detailed description
// can be found in "Introduction to algorithms" by Cormen, Leiserson and
// Rivest, MIT press (section 7.5 "priority queues").
//
// To use this priority queue, you must derive the things that you want to put
// into the queue from priorQelmt and redefine the priority() function. For
// example, the following Thing gets initialized with an integer that
// determines its priority in the queue:
//
//    class Thing: public priorQelmt
//    {
//       int prior;
//    public:
//       Thing(int thePriority) {prior = thePriority;} // initialize priority
//       virtual const int priority() {return prior;}  // redefine virtual func
//    }
//
// Then you can create a queue and insert Things into it:
//
//    Thing a(4), b(-12), c(19); // create three things w. priority 4, -12, 19
//    priorQ queue;              // create a priority queue named "queue"
//    queue.insert(a).insert(b).insert(c); // put three things in the queue
//
// The first Thing in the queue (this is the Thing with the highest value of
// the Thing::priority function) can then be obtained as follows:
//
//    Thing *firstThing = queue.extractFirst();
//
// This of course should extract the Thing with priority 19, that is: Thing c.
// You can intermix insert() and extractFirst() operations freely, without
// disturbing the correct operation of the priority queue.  You can also ask
// the queue to only _show_ what the first thing is, without extracting this
// thing from the queue:
//
//    Thing *firstThing = queue.first();
//
// Function qSize() tells how many things currently are in the priority queue:
//
//    int numberOfElementsInTheQueue = queue.qSize();
//
// Finally you can remove and even delete the elements from the priority queue:
//
//    queue.cleanup();
//

typedef enum { DoNotDeletePriorQelmts = 0, DeletePriorQelmts = 1 } priorQcleanupMode;

// priorQelmt serves as the base class that priorQ knows how to deal with. All
// things that you want to put in a priority queue must be derived from
// priorQelmt, and this derived class must redefine the virtual function
// priority() to reflect the priority that the derived object has in the
// context of the priority queue:
//
class priorQelmt
{
public:
   virtual const int priority() {return 0;} // the priority of this elmt
};

// priorQ is the priority queue class. It operates on a heap for which it
// allocates memory. If you know the maximum number of things that will ever be
// in the queue at the same time, you better specify this number as the first
// argument of the priorQ constructor. Whenever the heap turns out to be too
// small, priorQ calls increaseHeapSize() to reallocate memory for the heap.
// However, this is a time consuming operation (order O(heapSize)), so you
// better avoid it if you can. The default initial size of a priorQ is 256
// priorQelmts.
//
// The run time complexities of the queue operations are:
// heapify()          --> O(log heapSize)
// increaseHeapSize() --> O(heapSize)
// extractFirst()     --> O(log heapSize)
// insert()           --> O(log heapSize)
// first()            --> O(1)
//
class priorQ
{
private:
   priorQelmt **heap;		// the heap is an array of priorQelmt pointers
   int heapSize;		// the current size of the heap
   int maxHeapSize;		// the size of the allocated array
   const int parent(int i) {return i>>1;}        // floor(i/2)
   const int left(int i)   {return i<<1;}        // 2i
   const int right(int i)  {return (i<<1) + 1;}  // 2i + 1
   void exchange(int i, int j);	// exchange two entries in the heap
   void heapify(int i);
   void priorQerror(const char *);
   void increaseHeapSize();	// emergency: heap to small!

public:
   priorQ(int maxSize = 256);	// maxSize is only a hint
   ~priorQ();
   priorQelmt *extractFirst();	// extract elmt with highest priority from Q
   priorQ& insert(priorQelmt&);	// insert a new elmt into the Q
   const priorQelmt *first();	// return (not extract) elmt w. highest prior.
   const int qSize();		// number of elements in the queue
   void  cleanup(priorQcleanupMode = DoNotDeletePriorQelmts);
};
