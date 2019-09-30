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

#include <algorithm>
#include <new>

#include "src/space/libmin/minrobdd.h"

//------------------------------------------------------------------------------
// References
//
// [1] De Micheli, G., "Synthesis and optimization of digital circuits,"
//     McGraw-Hill, 1994.
//
//------------------------------------------------------------------------------

MINRobdd::MINRobdd(void)
{
        _variables.reserve(32);
        _nodes.reserve(128);

	MINRobddNode literal;
	literal.level = -1;
	literal.branch_0 = 0;
	literal.branch_1 = 0;

	// add false literal
	//
	_nodes.push_back(literal);

	// add true literal
	//
	_nodes.push_back(literal);
}

MINRobdd::~MINRobdd(void)
{
}

//------------------------------------------------------------------------------

// task: check if tuple (f,g,h) has already been computed by searching the
// 	 computed table; the return value is the index of the node, or -1 if not
// 	 found; see section 2.5.2. of [1] for further details;
//
STDSDWord
MINRobdd::findComputedEntry(STDDWord f, STDDWord g, STDDWord h) const
{
	MINRobddTuple key;
	key.v1 = f;
	key.v2 = g;
	key.v3 = h;

        Computed::const_iterator i = _computed.find(key);
        if(i == _computed.end())
            return -1;

        return i->second;
}

//------------------------------------------------------------------------------

// task: check if node (level,t,e) already exists; if not, a new node is created
//       and added to the unique table; the result is the index of the (possibly
//       new) node; see section 2.5.2. of [1] for further details;
//
STDDWord
MINRobdd::findOrAddUniqueTable(STDSDWord level, STDDWord t, STDDWord e)
{
	MINRobddTuple key;
	key.v1 = STDDWord(level);
	key.v2 = t;
	key.v3 = e;

        Unique::const_iterator i = _unique.find(key);
        if(i != _unique.end())
            return i->second;

	// create new node
	//
	MINRobddNode node;
	node.level = level;
	node.branch_0 = e;
	node.branch_1 = t;

	_nodes.push_back(node);

	// insert entry into unique table
	//
        _unique.insert(Unique::value_type(key, _nodes.size()-1));

	return _nodes.size()-1;
}

//------------------------------------------------------------------------------

// task: update the computed table with an entry {(f,g,h),r}; the key (f,g,h)
//       may not yet exist in the table;
// 	 see section 2.5.2. of [1] for further details;
//
void
MINRobdd::updateComputedTable(STDDWord f, STDDWord g, STDDWord h, STDDWord r)
{
	MINRobddTuple key;
	key.v1 = f;
	key.v2 = g;
	key.v3 = h;

	// STD_ASSERT(_computed.find(key) == _computed.end());
	_computed.insert(Computed::value_type(key, r));
}

//------------------------------------------------------------------------------

// task: determine the highest level of the nodes corresponding to the indices
//       f, g and h;
//
STDSDWord
MINRobdd::topVariable(STDDWord f, STDDWord g, STDDWord h) const
{
	STDSDWord level = _nodes[f].level;

	if(_nodes[g].level > level) level = _nodes[g].level;
	if(_nodes[h].level > level) level = _nodes[h].level;

	return level;
}

//------------------------------------------------------------------------------

// task: go one level below a specified level at a given node f, by taking the
//       branch specified by value; the index of the new node is returned;
//       if the level of node f is already less than the specified level, then
//       node f is returned;
//
STDDWord
MINRobdd::removeVariable(STDDWord f, STDSDWord level, bool value)
{
	STD_ASSERT(level >= 0);

	if(_nodes[f].level < level) return f;

	STD_ASSERT(_nodes[f].level == level);

	if(value)
		return _nodes[f].branch_1;
	else
		return _nodes[f].branch_0;
}

//------------------------------------------------------------------------------

// task: register a new variable with the robdd, assigning it the next level
//       which is still unoccupied; a new node corresponding to the variable
// 	 will be created; the return value is the index of this node; if the
//       variable is already registered, then this function simply does nothing
//       (except returning the corresponding node);
//
STDDWord
MINRobdd::symbolNode(const std::string& name)
{
        MINSymbol* symbol = symbol_table.find(name);
        if(symbol) return symbol->getDefinition();

        symbol = new MINSymbol(name);
	STD_MEMORY(symbol);
        symbol_table.insert(symbol);

	STDSDWord level = _variables.size();
	_variables.push_back(symbol);

	// note: add to unique table;
	//       (the find part of this call will always fail)
	//
	STDDWord f = findOrAddUniqueTable(level, 1, 0);

        symbol->define(f);
        return f;
}

//------------------------------------------------------------------------------

// task: define a new symbol name and associate it with an expression; the name
//       should not already exist

MINSymbol*
MINRobdd::defineSymbol(const std::string& name, STDDWord f)
{
    STD_ASSERT(symbol_table.find(name) == 0);

    MINSymbol* symbol = new MINSymbol(name, f);
    STD_MEMORY(symbol);
    symbol_table.insert(symbol);

    return symbol;
}

//------------------------------------------------------------------------------

// task: lookup a symbol

MINSymbol*
MINRobdd::findSymbol(const std::string& name) const
{
    return symbol_table.find(name);
}

//------------------------------------------------------------------------------

// task: perform the if-then-else operation; the algorithm resembles algorithm
//       2.5.2. shown on page 83 of [1];
//
STDDWord
MINRobdd::ite(STDDWord f, STDDWord g, STDDWord h)
{
	// note: check if this is a terminal case;
	//       see also page 81 of [1];
	//
	if(g == 1 && h == 0)
		return f;
	else if(f == 1)
		return g;
	else if(f == 0)
		return h;
	else if(g == h)
		return g;

	// note: check if node already computed
	//
	STDSDWord index = findComputedEntry(f, g, h);
	if(index >= 0)
		return index;

	// note: determine the topmost level of f, g, and h;
	//       the terminal case detection above assures that
	//       this level >= 0;
	//
	STDSDWord level = topVariable(f, g, h);
	STD_ASSERT(level >= 0);

	STDDWord f0 = removeVariable(f, level, false);
	STDDWord f1 = removeVariable(f, level, true);
	STDDWord g0 = removeVariable(g, level, false);
	STDDWord g1 = removeVariable(g, level, true);
	STDDWord h0 = removeVariable(h, level, false);
	STDDWord h1 = removeVariable(h, level, true);

	STDDWord t = ite(f1, g1, h1);
	STDDWord e = ite(f0, g0, h0);

	// note: check if both children reflect the same function
	//
	if(t == e) return t;

	STDDWord r = findOrAddUniqueTable(level, t, e);

	updateComputedTable(f, g, h, r);

	return r;
}

//------------------------------------------------------------------------------

// task: output the definition of node f as a nesting of ite functions;
//
ostream&
MINRobdd::doOutput(ostream& ostr, STDDWord f)
{
	if(f <= 1) return ostr << char('0' + f);

	STD_ASSERT(_nodes[f].level >= 0);

	ostr << "ite(" << _variables[_nodes[f].level]->get_name() << ", ";
	doOutput(ostr, _nodes[f].branch_1);
	ostr << ", ";
	doOutput(ostr, _nodes[f].branch_0);
	return ostr << ")";
}

//------------------------------------------------------------------------------

// task: determine which variables occur in a function;
//
void
MINRobdd::determineVariables(STDDWord f, STDIntSet& result) const
{
	STDIntSet visited;
        visited.reserve(_nodes.size());

	determineVariables(f, result, visited);
}

void
MINRobdd::determineVariables(STDDWord f, STDIntSet& result, STDIntSet& visited) const
{
	if(visited.contains(f))
		return;

	visited.insert(f);

	if(_nodes[f].level < 0)
		return;

	result.insert(_nodes[f].level);

	determineVariables(_nodes[f].branch_0, result, visited);
	determineVariables(_nodes[f].branch_1, result, visited);
}

//------------------------------------------------------------------------------

MINTable*
MINRobdd::createTable(STDDWord f, std::vector<MINSymbol*>& variables)
{
        variables.clear();

	// determine the variables
	//
	STDIntSet var_set;
        var_set.reserve(_variables.size());
	determineVariables(f, var_set);

	// remap levels to indices of fields within the table
	//
	std::vector<STDDWord> remap;
        remap.resize(_variables.size(), 0);

	STDDWord var = 0;
	for(STDDWord i = 0; i < _variables.size(); i++)
	{
		if(var_set.contains(i))
		{
			remap[i] = var++;

			// note: also build vector of used variables
			//
			variables.push_back(_variables[i]);
		}
	}

	std::vector<MINTable*> tables;
	tables.resize(_nodes.size(), 0);

	MINTable* table = createTable(f, var_set.get_cardinality(), remap, tables);

	// destroy vector of computed tables
	//
	tables[f] = 0;
	for(STDDWord i = 0; i < tables.size(); ++i)
	    if(tables[i]) delete tables[i];

	return table;
}

MINTable*
MINRobdd::createTable(STDDWord f, STDDWord num_vars,
		      const std::vector<STDDWord>& remap, std::vector<MINTable*>& tables)
{
	if(tables[f]) return tables[f];

	if(f == 0)
	{
		tables[f] = new MINTable(num_vars, 0);
		STD_MEMORY(tables[f]);
		return tables[f];
	}
	if(f == 1)
	{
		tables[f] = new MINTable(num_vars, 1);
		STD_MEMORY(tables[f]);
		tables[f]->addUniversalCube();
		return tables[f];
	}

	MINTable* table0 = createTable(_nodes[f].branch_0, num_vars, remap, tables);
	MINTable* table1 = createTable(_nodes[f].branch_1, num_vars, remap, tables);

	MINTable* table = new MINTable(num_vars, table0->getNumRows() + table1->getNumRows());
	STD_MEMORY(table);
	table->addRows(*table0);
	table->addRows(*table1);

	STDDWord row;
	STDDWord var = remap[_nodes[f].level];

	for(row = 0; row < table0->getNumRows(); row++)
		table->setField(row, var, 0x1);

	for(; row < table->getNumRows(); row++)
		table->setField(row, var, 0x2);

	tables[f] = table;
	return table;
}

//------------------------------------------------------------------------------

