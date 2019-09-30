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
#include "src/ocean/libocean/graph.h"
#include "src/ocean/libocean/set.h"

class derivedSetElmt : public setElmt
{
   int x;
public:
   derivedSetElmt(int xx) { x = xx; }
   virtual int ordinate() { return x; }
   virtual void print() { cout << "<" << x << ">"; }
};

main()
{
   // first test the set stuff...:
   set s1,
   s2(IncreasingOrderedSet, "increasing"),
   s3(DecreasingOrderedSet, "decreasing");

   derivedSetElmt *e1 = new derivedSetElmt(1);
   derivedSetElmt *e3 = new derivedSetElmt(3);
   derivedSetElmt *e5 = new derivedSetElmt(5);

   s1.add(e1)->add(e5)->add(e3);
   s1.print();
   s1.remove(e1)->remove(e3)->remove(e5);
   s2.add(e1)->add(e5)->add(e3);
   s2.print();
   s1.remove(e1)->remove(e3)->remove(e5);
   s3.add(e1)->add(e5)->add(e3);
   s3.print();
   setNextElmtIterator nextElmt(&s3);
   setPrevElmtIterator prevElmt(&s3);
   setElmt *elmt;
   while (elmt = nextElmt()) elmt->print();
   cout << "\n" << flush;
   while (elmt = prevElmt()) elmt->print();
   cout << "\n" << flush;

   // ...and now test the graph stuff...:
   graphDescriptor gd;
   gd.setCurrentGraph();
   printf ("sizeof graphElement = %d\n", sizeof(graphElement));
   graphVertex v1;
   graphVertex v2;
   graphVertex v3;
   graphEdge *e;
   graphEdge *edge1 = new graphEdge;
   graphEdge *edge2 = new graphEdge;
   edge1->addToEdge(&v1)->addToEdge(&v2)->addToEdge(&v3);
   edge2->addToEdge(&v2)->addToEdge(&v3);
   gd.print();
   cout << "\n" << flush;
   graphVertexEdgeIterator nextEdge(&v2);
   while (e = nextEdge()) e->print();
   cout << "\n" << flush;
   graphVertexCommonEdgeIterator nextCommonEdge (&v2, &v3);
   while (e = nextCommonEdge()) e->print();
   cout << "\n" << flush;
   graphVertex *v;
   graphVertexNeighborIterator nextNeighbor(&v1);
   while (v = nextNeighbor()) v->print();
   cout << "\n...and now the graphElementIterator:\n";
   graphElementIterator nextGraphElmt(&gd);
   graphElement *ge;
   while (ge = nextGraphElmt()) ge->printElement();
   cout << "\nvertices only:\n";
   nextGraphElmt.initialize (&gd, VertexGraphElementType);
   while (ge = nextGraphElmt()) ge->printElement();
   cout << "\n...edges only:\n";
   nextGraphElmt.initialize (&gd, EdgeGraphElementType);
   while (ge = nextGraphElmt()) ge->printElement();
   cout << "\n";
   edge1->removeFromEdge(&v1);
}
