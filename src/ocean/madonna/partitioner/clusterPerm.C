/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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
#include <stdlib.h>
#include "src/ocean/libocean/format.h"
#include "src/ocean/madonna/partitioner/matrixInt.h"
#include "src/ocean/madonna/partitioner/part.h"

// a connecInfo object enumerates the interconnections between clusters:
class connectInfo: private matrixInt
{
public:
   connectInfo (int numparts, CIRCUITPTR);
   int operator() (int pi, int pj); // #connections between partitions i and j
   int numparts() { return dimension1(); }
};

// a distInfo object enumerates the distance between two clusters:
class distInfo: private matrixInt
{
public:
   distInfo (int nx, int ny);
   int operator() (int pi, int pj); // distance between partition i and j
   void exchange (int pi, int pj);  // exchange partitions i and j
   int numparts() { return dimension1(); }
};

int connectInfo::operator() (int pi, int pj)
{
   matrixInt& mtx = (matrixInt&) *this;
   return mtx[pi][pj];
}

// initialize the connectivity matrix from the netlist of CIRCUIT:
connectInfo::connectInfo (int numparts, CIRCUITPTR circuit) : matrixInt (numparts, numparts)
{
   int *row = new int[numparts];
   matrixInt& mtx = (matrixInt&) *this;
   for (NETPTR net = circuit->netlist; net; net = net->next)
   {
      int j;
      for (j = 0; j < numparts; ++j) row[j] = 0;
      // now mark all partitions that this net connects to:
      for (CIRPORTREFPTR cpr = net->terminals; cpr; cpr = cpr->next)
      {
	 if (!cpr->cirinst) continue; // terminal on top circuit
	 int partition = atoi (cpr->cirinst->name) - 1;
	 row[partition] = 1;	// mark this partition
      }
      //
      // row[] is now a boolean vector with a 1 for each partition that is
      // visited by this net. Now add the matrix row*Transpose(row) to mtx:
      for (int i = 0; i < numparts; ++i)
      {
	 if (row[i] == 0) continue; // this net is not in partition i
	 for (j = 0; j < numparts; ++j)
	    mtx[i][j] += row[j]; // account for this net
      }
   }
   for (int ij = 0; ij < numparts; ++ij) mtx[ij][ij] = 0; // reset diagonal
   delete row;
}

// return the distance between partitions pi and pj.
int distInfo::operator() (int pi, int pj)
{
   matrixInt& mtx = (matrixInt&) *this;
   return mtx[pi][pj];
}

// exchange the partitions pi and pj. This means that the distances from pj to
// any other partition are the distances that currently pi has to these other
// partitions, and vice versa. So we can just exchange the columns pi and pj,
// and exchange the rows pi and pj (order is unimportant).
void distInfo::exchange (int pi, int pj)
{
   exchangeRows (pi, pj);
   exchangeColumns (pi, pj);
}

static inline int absolute (int x) { if (x < 0) return -x; else return x; }

// create a matrix[numparts][numparts] and init each entry (i,j) with the
// distance from partition i to partition j:
distInfo::distInfo (int nx, int ny) : matrixInt (nx * ny, nx * ny)
{
   for (int i = 0; i < numparts(); ++i)
   {
      int yi = i % nx;
      int xi = i / nx;
      for (int j = 0; j < numparts(); ++j)
      {
	 int yj = j % nx;
	 int xj = j / nx;
	 int dist = absolute (xi - xj) + absolute (yi - yj);
	 matrixInt& mtx = (matrixInt&) *this;
	 mtx[i][j] = dist;
      }
   }
}

// Compute some measure of the current cluster cost. The lower this number, the
// better the clustering. Cannot do better then zero!
int clusterCost (connectInfo& connectivity, distInfo& distance)
{
   int cost = 0;
   int numParts = connectivity.numparts();
   for (int i = 0; i < numParts - 1; ++i)
      for (int j = i + 1; j < numParts; ++j)
	 cost += connectivity (i,j) * distance (i,j);
   return cost;
}

void applyPermutation (connectInfo& connectivity, distInfo& distance,
		      int a, int b, int *pi,
		      int& bestCost, int bestPerm[], int& worstCost)
{
   if (a == b) return;
   a -= 1; b -= 1; pi += 1;	// Trotter-Johnson use range 1..n NOT 0..n-1
   distance.exchange (a, b);	// exchange the partitions a and b
   int thisCost = clusterCost (connectivity, distance);
   if (thisCost < bestCost)
   {
      bestCost = thisCost;
      int permsize = connectivity.numparts();
      for (int i = 0; i < permsize; ++i)
	 bestPerm[i] = pi[i];	// save this permutation
   }
   if (thisCost > worstCost) worstCost = thisCost;
}

// This is a "minimal change" algorithm by Trotter and Johnson to generate all
// permutations of the sequence 1,2,..,n. See Reingold, Nievergelt and Narsing
// Deo, "Combinatorial Algorithms", Prentice Hall, 1977.
// It is a minimum change algorithm because it only uses the "exchange two
// neighbors" operation to permutate the order of the sequence.
// On some computer the function perm() generates about 1,250,000 permutations
// per second, that is, perm(10) takes about 3 seconds to complete.
//
void generatePermutations (int& bestCost, int bestPerm[], int& worstCost,
			  connectInfo& connectivity, distInfo& distance)
{
   int n = distance.numparts();
   int m = n+1;
   // pi[1..n] is "active", pi[0] and pi[n+1] are auxiliaries
   int *pi = new int[n+2];
   int *p  = new int[n+2];
   int *d  = new int[n+2];

   for (int i = 1; i <= n ; ++i)
   {
      pi[i] = p[i] = i;
      d[i] = -1;
   }
   d[1] = 0;
   pi[0] = pi[n+1] = m;
   register int a = 0, b = 0;
   while (m > 1)
   {
      applyPermutation (connectivity, distance, a, b, pi,
		       bestCost, bestPerm, worstCost);
      m = n;
      while (pi[p[m]+d[m]] > m)
      {
	 d[m] = -d[m];
	 --m;
      }
      // exchange the two neighbors a and b:
      a = p[m];
      b = p[m] + d[m];
      register int temp = pi[a];
      pi[a] = pi[b];
      pi[b] = temp;
      // exchange some more things:
      temp = p[pi[p[m]]];
      p[pi[p[m]]] = p[m];
      p[m] = temp;
   }
   delete pi, p, d;
}

void permutateThisPartitioning (TOTALPPTR total, int bestPerm[])
{
   CIRCUITPTR  c = total->bestpart;
   if (!c) return;
   for (CIRINSTPTR cinst = c->cirinst; cinst; cinst = cinst->next)
   {
      int oldpartition = atoi (cinst->name);
      int newpartition;
      for (newpartition = 0; newpartition < total->numparts; ++newpartition)
	 if (bestPerm[newpartition] == oldpartition)
	    break;
      if (newpartition >= total->numparts)
	 err (5, (char*)"permutate: internal error 5675");
      newpartition += 1;	// since we start counting from 1, not 0 ...
      fs (cinst->name);
      cinst->name = cs (form ("%d", newpartition));
   }
}

#define INF_COST 1000000

int clusterPermutate (TOTALPPTR total)
{
   if (!total->bestpart) return 0;
   if (total->numparts > 8) return 0; // later I think of something smarter
   if (total->numparts < 3) return 0; // less than 3 does not make sense ...
   connectInfo connectivity (total->numparts, total->bestpart);
   distInfo distance (total->nx, total->ny);
   int startCost = clusterCost (connectivity, distance);
   int bestCost = INF_COST, worstCost = -INF_COST;
   int *bestPerm = new int[total->numparts];
   generatePermutations (bestCost, bestPerm, worstCost, connectivity, distance);
   cout << form ("------ s, w, b = %d, %d, %d; perm = ", startCost, worstCost, bestCost);
   for (int i = 0; i < total->numparts; ++i) cout << bestPerm[i] << " ";
   cout << endl;
   if (bestCost != INF_COST) permutateThisPartitioning (total, bestPerm);
   return 0;
}
