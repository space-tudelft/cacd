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
#ifndef __COMP_VISITORS_H__
#define __COMP_VISITORS_H__

// STL includes
#include <string>
#include <vector>

using namespace std;

// Forward declarations
class CComponent;

//! This class is allowed to visit CComponent and CComponentTree objects.
/*!
    The CComponentVisitor class is an abstract base class that
    defines the interface used to visit CComponent and CComponentTree
    objects.

    The visitor class is an important class in the visitor pattern as
    described by Erich Gamma and others in their excellent book
    'Design Patterns'.

    \sa CFindPropertyVisitor, CFindComponentVisitor, CDumpComponentTreeVisitor
    \author Xander Burgerhout
*/
class CComponentVisitor
{
    public:
	CComponentVisitor() {};
	virtual ~CComponentVisitor() {};
	virtual void visitComponent(CComponent* component) = 0;
};

//! Finds components in a component tree
/*!
    This CComponentVisitor derived class searches for a component with a given
    name. The name can be just the name of the component, a part of the
    full-context name or the entire full-context name.

    All matches found are stored for later access.

    \sa CFindPropertyVisitor, CComponentVisitor
    \author Xander Burgerhout
*/
class CFindComponentVisitor : public CComponentVisitor
{
    private:
	string              m_searchName;   //<! The name of the component to find
	vector<CComponent*> m_hits;         //<! The components found

	bool                isAMatch(const string& src, const string& full);
	bool                containsComponent(CComponent* comp);

    public:
	CFindComponentVisitor(const string& name);
	CFindComponentVisitor(const CFindComponentVisitor& src);

	void                visitComponent(CComponent* component);
	int                 hitCount();
	CComponent*         getHit(int index = 0);
	void                reset();

	CFindComponentVisitor& operator=(const CFindComponentVisitor& src);
};

//! Dumps debug information
/*!
    This CComponentVisitor derived class dumps the structure
    of the component tree being visited to stderr.

    \sa CComponentTree::dump(), CComponent::dump()
    \author Xander Burgerhout
*/
class CDumpComponentTreeVisitor // : public CComponentVisitor
{
    public:
	string              m_leadSpace;    //!< Current indentation
	void                visitComponent(CComponent* comp);
};

//! Finds components with a certain property in a component tree
/*!
    This CComponentVisitor derived class searches for components that have
    a certain property with a certain value.

    <ul>
    <li>It is possible to specify if the property value should be equal or not
    equal to the desired value. </li>

    <li>If the properties value is a vector, only the first value is compared.</li>

    <li>All matches found are stored for later access.</li>
    </ul>

    \sa CFindPropertyVisitor, CComponentVisitor
    \author Xander Burgerhout
*/
class CFindPropertyVisitor : public CComponentVisitor
{
    private:
	int                 m_comparison;   //!< The type of comparison to perform
	string              m_property;     //!< The property to look for
	string              m_value;        //!< The value to compare with
	vector<CComponent*> m_hits;         //!< Contains the components matching the criteria

    public:
	enum {
	    EQUAL,      //!< Comparison criterium is equal to
	    NOTEQUAL    //!< Comparison criterium is not equal to
	};

	CFindPropertyVisitor(const string& property, const string& value, int comparison = EQUAL);

	void                visitComponent(CComponent* comp);
	int                 hitCount();
	CComponent*         getHit(int index = 0);
	void                reset();
};

#endif // __COMP_VISITORS_H__
