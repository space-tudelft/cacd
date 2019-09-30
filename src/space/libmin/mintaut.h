/*
 * ISC License
 *
 * Copyright (C) 1997-2018 by
 *	Arjan van Genderen
 *	Kees-Jan van der Kolk
 *	Simon de Graaf
 *	Nick van der Meijs
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

#ifndef __MINTAUT_H__
#define __MINTAUT_H__

#include 	<iostream>

#include 	"src/space/libmin/mindefs.h"
#include 	"src/space/libmin/STDIntSet.h"
#include 	"src/space/libmin/minsolve.h"

using namespace std;
using namespace libmin;

//------------------------------------------------------------------------------

class MINTautologySolver : public MINSolver
{
        STD_DECLARE_CLASS(MINTautologySolver);

protected:
	// set of variables in which the function is unate
	//
	STDIntSet 		_unate;

public:
	// ctor/dtor
	//
        MINTautologySolver(const MINTable& table);
        ~MINTautologySolver(void);

protected:
	// classify variables
	//
	void 			reclassifyVars(void);

	// store a leaf into a cover matrix (may be used by derived classes)
	//
	virtual void 		storeLeaf(void);

public:
	// output
	//
	ostream& 		doOutput(ostream& ostr) const;
	friend ostream& 	operator<<(ostream& ostr, const MINTautologySolver& solver);

	// solve
	//
	bool 		solve(void);
};

//------------------------------------------------------------------------------

class MINTautologyCoverSolver : public MINTautologySolver
{
private:
	// cover to fill
	//
	MINCover& 		_cover;

	// the implicant alpha which is currently being examined
	//
	STDDWord 		_alpha;

	// the number of implicants in the relatively essential set
	//
	STDDWord 		_er_size;

public:
	// ctor/dtor
	//
				MINTautologyCoverSolver(const MINTable& table,
							MINCover& cover,
							STDDWord alpha,
							STDDWord er_size);
				~MINTautologyCoverSolver(void);

public:
	// store leaf
	//
	void 			storeLeaf(void);

	// remove rows
	//
	void 			removeRows(const STDIntSet& rows);
};

//------------------------------------------------------------------------------

#endif
