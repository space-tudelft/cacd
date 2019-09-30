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

#ifndef __MINROBDD_H__
#define __MINROBDD_H__

#include <iostream>
#include <vector>
#include <map>

#include 	"src/space/libmin/mindefs.h"
#include 	"src/space/libmin/mintable.h"
#include 	"src/space/libmin/minsym.h"

using namespace std;

//------------------------------------------------------------------------------

struct MINRobddNode
{
        STD_DECLARE_CLASS(MINRobddNode);

public:
	// level (-1 in case of literal)
	//
	STDSDWord 	level;

	// 0/1 branches
	//
	STDDWord 	branch_0;
	STDDWord	branch_1;
};

//------------------------------------------------------------------------------

struct MINRobddTuple
{
        STD_DECLARE_CLASS(MINRobddTuple);

public:
	STDDWord v1;
	STDDWord v2;
	STDDWord v3;

	bool operator==(const MINRobddTuple& tuple) const
	{
		return v1 == tuple.v1 && v2 == tuple.v2 &&
		       v3 == tuple.v3;
	}

        bool operator<(const MINRobddTuple& other) const
        {
            if(v1 < other.v1) return true;
            if(v1 > other.v1) return false;
            if(v2 < other.v2) return true;
            if(v2 > other.v2) return false;
            if(v3 < other.v3) return true;
            if(v3 > other.v3) return false;
            return false;
        }

	STDDWord hash(void) const
	{ return v1^(v2<<4)^(v3<<8); }
};

//------------------------------------------------------------------------------

class MINRobdd
{
        STD_DECLARE_CLASS(MINRobdd);

private:
        // Symbol table.
        //
        MINSymbolTable symbol_table;

	// variables (ordered by level)
	//
	std::vector<MINSymbol*> _variables;

	// vector of nodes
	//
	// note: element 0 represents the false literal
	//       element 1 represents the true  literal
	//
	std::vector<MINRobddNode> _nodes;

	// table of computed nodes
	//
	typedef std::map<MINRobddTuple,STDDWord> Computed;
	Computed _computed;

	// unique table
	//
	typedef std::map<MINRobddTuple,STDDWord> Unique;
        Unique _unique;

public:
	// ctor/dtor
	//
			MINRobdd(void);
			~MINRobdd(void);

private:
	// find computed entry
	// note: returns -1 if not found
	//
	STDSDWord 	findComputedEntry(STDDWord f, STDDWord g, STDDWord h) const;

	// find or add to unique table
	//
	STDDWord 	findOrAddUniqueTable(STDSDWord level, STDDWord t, STDDWord e);

	// update computed table
	//
	void 		updateComputedTable(STDDWord f, STDDWord g, STDDWord h, STDDWord r);

	// find top variable
	//
	STDSDWord 	topVariable(STDDWord f, STDDWord g, STDDWord h) const;

	// remove a variable
	//
	STDDWord 	removeVariable(STDDWord f, STDSDWord level, bool value);

	// determine variables
	//
	void 		determineVariables(STDDWord f, STDIntSet& result) const;
	void 		determineVariables(STDDWord f, STDIntSet& result, STDIntSet& visited) const;

	// create table, using list of already computed tables
	//
	MINTable* 	createTable(STDDWord f, STDDWord num_vars,
				const std::vector<STDDWord>& remap, std::vector<MINTable*>& tables);

public:
        // Return node for a new/existing symbol.
        //
        STDDWord        symbolNode(const std::string& name);

        // Define a new symbol (and associate it with a definition).
        //
        MINSymbol*      defineSymbol(const std::string& name, STDDWord f);

        // Find a symbol.
        //
        MINSymbol*      findSymbol(const std::string& name) const;

	// if-then-else algorithm
	//
	STDDWord 	ite(STDDWord f, STDDWord g, STDDWord h);

	// create table
	//
	MINTable* 	createTable(STDDWord f, std::vector<MINSymbol*>& variables);

public:
	// return number of nodes (used for diagnostic purposes)
	//
	STDDWord 	getNumNodes(void) const;

	// output
	//
	ostream& 	doOutput(ostream& ostr, STDDWord f);
};

//------------------------------------------------------------------------------

inline STDDWord
MINRobdd::getNumNodes(void) const
{
	return _nodes.size();
}

//------------------------------------------------------------------------------

#endif
