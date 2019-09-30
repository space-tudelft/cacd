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

#include 	"src/space/libmin/minsym.h"

//==============================================================================
//
//  MINSymbol methods.
//
//==============================================================================

MINSymbol::MINSymbol(const std::string& name)
	: _name(name), _definition(-1)
{
}

MINSymbol::MINSymbol(const std::string& name, STDSDWord definition)
	: _name(name), _definition(definition)
{
	STD_ASSERT(definition >= 0);
}

MINSymbol::~MINSymbol(void)
{
}

//------------------------------------------------------------------------------

// task: define a symbol; definition may be the number of the corresponding node
//       in a ROBDD;
//
void
MINSymbol::define(STDDWord definition)
{
	STD_ASSERT(_definition < 0);
	_definition = definition;
}

//------------------------------------------------------------------------------

// task: return the definition of a symbol;
//
STDSDWord
MINSymbol::getDefinition(void) const
{
	return _definition;
}


//==============================================================================
//
//  MINSymbolTable methods.
//
//==============================================================================

MINSymbolTable::MINSymbolTable(void)
{
}

MINSymbolTable::~MINSymbolTable(void)
{
    for(SymbolMap::iterator i = symbol_map.begin(); i != symbol_map.end(); ++i)
        delete i->second;
}

void
MINSymbolTable::insert(MINSymbol* symbol)
{
    STD_ASSERT(symbol_map.find(symbol->get_name()) == symbol_map.end());
    symbol_map.insert(SymbolMap::value_type(symbol->get_name(), symbol));
}

MINSymbol*
MINSymbolTable::find(const std::string& name) const
{
    SymbolMap::const_iterator i = symbol_map.find(name);
    if(i == symbol_map.end())
        return 0;

    return i->second;
}

