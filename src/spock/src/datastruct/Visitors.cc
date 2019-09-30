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

// Qt includes
#include <qregexp.h>

// Project includes
#include "src/spock/src/datastruct/Component.h"
#include "src/spock/src/datastruct/Visitors.h"

// STL includes
#include <sstream>
#include <string>

using namespace std;


//============================================================================
//! default constructor
/*! \param name is the name of the component to find. Name can be any
    of the following:
    <ul>
        <li> Just the name of the component </li>
        <li> Part of the full-context name, e.g. C.D </li>
        <li> The full-context name, e.g. A.B.C.D </li>
    </ul>
 */
CFindComponentVisitor::CFindComponentVisitor(const string& name)
    : CComponentVisitor()
{
    m_searchName = name;
}

//============================================================================
//! The copy constructor
/*! \param src the CFindComponentVisitor to copy */
CFindComponentVisitor::CFindComponentVisitor(const CFindComponentVisitor& src)
    : CComponentVisitor()
{
    *this = src;
}

//============================================================================
//! Checks if \a src is part of \full.
/*!
    \param src  the string to match
    \param full the string to match against

    \return true if the matching is succesfull, false if not.
*/
bool CFindComponentVisitor::isAMatch(const string& src, const string& full)
{
    string regexpr = "\\." + src + "$";
    QRegExp regular(regexpr.c_str());
    if (regular.match(full.c_str()) != -1) return true;

    regexpr = "^" + src + "$";
    QRegExp regular2(regexpr.c_str());
    if (regular2.match(full.c_str()) != -1) return true;
    return false;
}

//============================================================================
//! Checks if \a component was already found
/*! \param component the component to check for
    \return true if \a component was already found, false otherwise
*/
bool CFindComponentVisitor::containsComponent(CComponent* component)
{
    for (unsigned int i=0; i<m_hits.size(); ++i) {
        if (component == m_hits[i])
            return true;
    }
    return false;
}

//============================================================================
//! Checks if \a component is a match
/*! If \a component is a match, it is added to the results
*/
void CFindComponentVisitor::visitComponent(CComponent* component)
{
    string name = component->getFullContextName();
    if (isAMatch(m_searchName, name)) {
        if (!containsComponent(component))
            m_hits.push_back(component);
    }
}

//============================================================================
//! Returns the number of components found that match the criterium
/*! \return The number of components found that match the criterium
*/
int CFindComponentVisitor::hitCount()
{
    return (int)m_hits.size();
}

//============================================================================
//! Returns hit number \a index
/*! \param index The index of the hit to return. If this parameter is not
                 specified, the first hit is returned.
    \return The component found by hit nr \a index. If no components were
            found or if \a index is out of bounds, 0 is returned.
*/
CComponent* CFindComponentVisitor::getHit(int index /* = 0 */)
{
    if (index>=0 && (unsigned)index<m_hits.size())
        return m_hits[index];
    return 0;
}

//============================================================================
//! Resets this query so a new query can be performed.
void CFindComponentVisitor::reset()
{
    m_hits.clear();
}

//============================================================================
//! Assigment operator
CFindComponentVisitor& CFindComponentVisitor::operator=(const CFindComponentVisitor& src)
{
    m_searchName = src.m_searchName;
    m_hits       = src.m_hits;
    return *this;
}

//============================================================================
//! Dumps debug information about \a comp
/*! \param comp The component to visit and dump information about */
void CDumpComponentTreeVisitor::visitComponent(CComponent* comp)
{
    ostringstream tmp;
    tmp << m_leadSpace << "- type = " << comp->getType() << endl;
    tmp << m_leadSpace << "- numChildren = " << comp->numChildren() << endl;
    tmp << ends;
    debug("%s", tmp.str().c_str());
}

//============================================================================
//! default constructor
/*! \param property is the property of the component to find.
    \param value    is the value the properties values should be compared with.
                    If the property being compared has vetored values, only the
                    first value in the vector will be compared.
    \param comparison the comparison criterium. Can be any of the following:
    <ul>
        <li> EQUAL if the value found should be equal to the property's value </li>
        <li> NOTEQUAL if the value found should be not equal to the property's value </li>
    </ul>
 */
CFindPropertyVisitor::CFindPropertyVisitor(const string& property, const string& value, int comparison)
    : CComponentVisitor()
{
    m_comparison = comparison;
    m_property = property;
    m_value = value;

    /*
    debug("You want to check all components for property '%s' and a value of '%s'", property.c_str(), value.c_str());
    if (comparison == EQUAL)
        debug("You want it to be a hit if the value matches.");
    else
        debug("You want it to be a hit if the value does not match.");
    */
}

//============================================================================
//! Checks if \a comp is a match
/*! If \a comp is a match, it is added to the results
*/
void CFindPropertyVisitor::visitComponent(CComponent* comp)
{
    ASSERT(comp != 0);

    string value = comp->getProperty(m_property);

    // debug("Considering %s (%s)", value.c_str(), comp->getFullContextName().c_str());

    if (m_comparison == EQUAL) {
        if (m_value == value)
            m_hits.push_back(comp);
    }
    if (m_comparison == NOTEQUAL) {
        if (m_value != value)
            m_hits.push_back(comp);
    }
}

//============================================================================
//! Returns the number of components found that match the criterium
/*! \return The number of components found that match the criterium
*/
int CFindPropertyVisitor::hitCount()
{
    return (int)m_hits.size();
}

//============================================================================
//! Returns hit number \a index
/*! \param index The index of the hit to return. If this parameter is not
                 specified, the first hit is returned.
    \return The component found by hit nr \a index. If no components were
            found or if \a index is out of bounds, 0 is returned.
*/
CComponent* CFindPropertyVisitor::getHit(int index /* = 0 */)
{
    if (index>=0 && (unsigned)index<m_hits.size())
        return m_hits[index];
    return 0;
}

//============================================================================
//! Resets this query so a new query can be performed.
void CFindPropertyVisitor::reset()
{
    m_hits.clear();
}
