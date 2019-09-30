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

//				   * * * * *
// NOTE: the hash table classes below really need an implementation based on
// templates, rather than the messy way it is implemented at this moment. It
// seems impossible to define an elegant hash table without knowing in advance
// what the type of the search key is. Templates solve this problem.
//				   * * * * *
//
// This implements the classes hashTable and hashTableElmt that realize a hash
// table. To use it, you must derive the things that you want to put into the
// hash table from hashTableElmt. For example, suppose you want to hash Things
// that have a certain integer value, then this could be the definition of such
// a Thing:
//
//    class Thing: public hashTableElmt
//    {
//    private:
//       int value;
//    public:
//       Thing(int v) {value = v;}
//       const int val() {return value;}
//    };
//
// The tricky part is in the hashTable class itself. Because we want to allow
// any type of search key, the hashTable base class cannot deal with this
// search key directly, since its type is unknown. Therefore the base class
// only defines three "builtin" functions for builtinInsert(), builtinSearch()
// and builtinRemove() that must be called from the derived class. This derived
// class must also define the virtual function hasTheRightKey(), for example in
// the following way:
//
//    class myHashTab: public hashTable
//    {
//    private:
//       int thekey; // for cummunication from search() to hasTheRightKey()
//       // I do not claim this is a great hash function, but alas ...:
//       unsigned long hashFunction(int v) {return (v << 19) + (v << 27) + v;}
//    public:
//       // constructor only passes the table size to the hashTable base class:
//       myHashTab(unsigned long size): hashTable(size) {};
//
//       // insert() puts the thing into the hash table:
//       myHashTab& insert(Thing &t) {
//          hashkey = hashFunction(t.val());
//          builtinInsert(t);
//          return *this;
//       }
//
//       // search() returns the thing in the hash table with the right key:
//       Thing *search(int key) {
//          thekey = key; // builtinSearch calls hasTheRightKey()
//          hashkey = hashFunction(key);
//          return (Thing *)builtinSearch();
//       }
//
//       // remove() deletes the specified thing from the hash table:
//       myHashTab& remove(Thing &t) {
//          hashkey = hashFunction(t.val());
//          builtinRemove(t);
//          return *this;
//       }
//
//       // this one is called by builtinSearch() which is called by search():
//       virtual const int hasTheRightKey(hashTableElmt& elmt)
//       {  // return TRUE if the search key of elmt equals thekey:
//          return ((Thing &)elmt).val() == thekey;
//       }
//    };
//
// The value returned by hashFunction() should range from 0 to <very big>,
// where <very big> means "many times larger then the hash table size".  Having
// myHashTab defined, you can create a hash table and put Things in it:
//
//       Thing a(12), b(-15), c(128); // create 3 things with values 12,-15,128
//       myHashTab ht(1021);          // create a hashtable with 1021 slots
//       ht.insert(a).insert(b).insert(c); // insert the three things into it
//
// Note that you have to specify the size of the hash table when you create it.
// A good size, for example, is 2 times the number of Things that you are going
// to put into the hash table. For a good hash table performance, it is very
// important that the size is a PRIME NUMBER. For instance, if you want your
// hash table to have 2000 slots, type "primes 2000 2020" on the command line
// of a unix computer, then pick the first prime that it prints, that is 2003.
// If you put more things in the hash table than the specified size, the hash
// table still works but the ht.search() and ht.delete() functions start to
// become a little bit slower.
//
// Now you can check that Thing b is in the hash table:
//
//      Thing *someThing = ht.search(-15);
//      if (someThing == NULL)
//         cout << "I put it in, but now it is not there anymore!\n";
//      else
//         cout << "found Thing with value " << someThing->val() << endl;
//
// This code of course should print "found Thing with value -15".
//

#ifndef __HASTABLE_H
#define __HASTABLE_H

// hashtableElmt is the base class for things that must be put in a hash table.
//
class hashTableElmt
{
private:
   hashTableElmt *nextElmt;
   friend class hashTable;
   friend class hashTableSlot;
public:
   hashTableElmt();
   virtual ~hashTableElmt();
};

typedef enum {
    DoNotDeleteHashTableElmts = 0,
    DeleteHashTableElmts = 1
} hashCleanupMode;

// do not use the hashTableSlot class, it is for private use by the hashTable
// class only:
//
class hashTableSlot
{
private:
   hashTableElmt *chain;
   hashTableSlot *nextUsedSlot;
   friend class hashTable;
public:
   // There is no public constructor or destructor for a hashTableSlot because
   // of run-time efficiency. The hashTable class takes care of this in a smart
   // way, using the list of usedSlots and taking into account the hashOptions.

   // unlink all hash table elmts from this slot:
   void cleanup(hashCleanupMode =DoNotDeleteHashTableElmts);
};

// hashTable is the class that represents a hash table. You can only put
// hashTableElmts in this hash table.
//
class hashTable
{
private:
   hashTableSlot *usedSlots;	         // list of slots in use
   hashTableSlot *lastUsedSlot;		 // most recently used slot
   hashTableSlot *table;		 // the hash table itself
   unsigned long tableSize;
   unsigned long numberOfUsedSlots, numberOfElmts; // statistics
   unsigned options;				   // hash options
   void hashError(const char *s);
protected:
   unsigned long hashkey;
public:
   hashTable(unsigned long hashTableSize, unsigned hashOptions =0, ...);
   hashTable&    builtinInsert(hashTableElmt&); // put new elmt in the table
   hashTable&    builtinRemove(hashTableElmt&); // delete element from the table
   hashTableElmt *builtinSearch();              // look-up element in the table
   // clear the hashTable by unlinking all the chains in the slots:
   void          cleanup(hashCleanupMode =DoNotDeleteHashTableElmts);
   virtual const int hasTheRightKey(hashTableElmt& e) = 0;
};

#endif
