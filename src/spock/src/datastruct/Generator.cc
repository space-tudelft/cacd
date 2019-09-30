/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Xander Burgerhout
 *	Simon de Graaf
 *	N.P. van der Meijs
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

// Project includes
#include "src/spock/src/datastruct/Generator.h"

using namespace std;

//============================================================================
//! Returns the context for this value mapping.
/*! \return The context for set for this value map.
    \sa CGeneratorComp::getMapped()
 */
string CValueMap::getContext() const
{
    return m_context;
}

//============================================================================
//! Sets the context for this mapping.
/*! \param context  The new context for this mapping.
    \sa getContext()
 */
void CValueMap::setContext(const string& context)
{
    m_context = context;
}

//============================================================================
// Maps a value
/*! \param from     Value to map from. This is the text as it would be
                    generated without the mapping.
    \param to       Value to map to. This is the replacement text.
*/
void CValueMap::makeMap(const string& from, const string& to)
{
    m_mapping[from] = to;
}

//============================================================================
//! Returns the replacement text for \a src.
/*! \param src The text that needs to be replaced.
    \return The replacement text for \a src. If \a src was not found in the
    mapping, the empty string is returned ("").
*/
string CValueMap::getMapped(const string& src)
{
    //debug("Looking up %s...", src.c_str());
    string lookup = src; //context + "." + src;
    if (m_mapping.count(lookup)>0) {
        //debug("found it, returning %s!", m_mapping[lookup].c_str());
        return m_mapping[lookup];
    }

    //debug("Not found in mappping!");

    //map<string, string>::iterator pos;
    //for (pos = m_mapping.begin(); pos != m_mapping.end(); ++pos)
    //    debug("%s\t-> %s", pos->first.c_str(), pos->second.c_str());
    return src;
}

//============================================================================
//! Constructor
/*! Calls the base class constructor (CComponent::CComponent()).
    \param parent   The parent component. Defaults to 0 (no parent).
    \param name     The name for this component. Defaults to "Unnamed".

    \sa CComponent, CComponent::CComponent().
*/
CGeneratorComp::CGeneratorComp(CComponent* parent /*=0*/, const string& name /*="Unnamed"*/)
    : CComponent(parent, name)
{
}

//============================================================================
//! Returns the number of value maps registered by this generator.
/*! \return The number of value maps registered by this generator.
    \sa getMapping(), addMapping()
*/
int CGeneratorComp::numMaps() const
{
    return (const int)m_mappings.size();
}

//============================================================================
//! Returns value map nr \a index.
/*! \return value map nr \a index. or 0 if index was out of bounds. */
CValueMap* CGeneratorComp::getMapping(int index) const
{
    if (index>=0 && (unsigned)index<m_mappings.size())
        return m_mappings[index];
    return 0;
}

//============================================================================
//! Adds a value map.
/*! \param newMap the new mapping to add to the list of mappings. Double
                  occurances are not checked.
    \sa getMapping()
*/
void CGeneratorComp::addMapping(CValueMap* newMap)
{
    m_mappings.push_back(newMap);
}

//============================================================================
//! Returns the mapped value of \a item.
/*!
    The list of value maps is searched for the right context, which is
    specified by \a type. Then, CValueMap::getMapped() is called to retrieve the
    mapped value of \a item.

    \param type     Used for retrieving the right CValueMap.
    \param item     The item to translate.
 */
string CGeneratorComp::getMapped(const string& type, const string& item)
{
//    debug("%s, %s", type.c_str(), item.c_str());
    for (int i=0; i<numMaps(); ++i) {
        if (type == getMapping(i)->getContext())
            return getMapping(i)->getMapped(item);
    }
    return item;
}

//============================================================================
//! Default constructor.
CGenerators::CGenerators()
{
}

//============================================================================
//! Adds a generator component
/*!
    Adds a generator to the collection of generators.
    If the generator is already present the method does nothing.
    \param comp The CGeneratorComp to add.
    \sa numGenerators(), getGenerator()
 */
void CGenerators::addGeneratorComp(CGeneratorComp* comp)
{
    if (comp != 0) {
        bool found = false;
        for (unsigned int i=0; i<m_generators.size(); ++i) {
            if (m_generators[i] == comp)
                found = true;
        }
        if (!found)
            m_generators.push_back(comp);
    }
}

//============================================================================
//! Returns the number of generators contained in this collection of generators.
/*! \return the number of generators contained in this collection of generators.
    \sa addGeneratorComp(), getGenerator()
 */
int CGenerators::numGenerators()
{
    return (int)m_generators.size();
}

//============================================================================
//! Returns the generator at \a index.
/*! \return the generator at \a index. Returns 0 if index is out of bounds.
    \sa addGeneratorComp(), numGenerators()
 */
CGeneratorComp* CGenerators::getGenerator(int index)
{
    if (index>=0 && (unsigned)index<m_generators.size())
        return m_generators[index];
    return 0;
}
