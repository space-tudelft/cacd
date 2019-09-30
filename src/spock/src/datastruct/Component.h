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
#ifndef __COMPONENT_H__
#define __COMPONENT_H__
//============================================================================
//! This file contains the CComponent class.
/*! \file
    The CComponent class is used as a building block in the component tree and
    generator tree built by the parser.

    \sa CParser, CComponentTree, CGenerator

    \author Xander Burgerhout
*/
//============================================================================

// STL includes
#include <vector>
#include <map>
#include <string>

using namespace std;

// Qt includes
#include <qdatastream.h>

// Some forward declarations
class QWidget;
class QLayout;

class CComponentVisitor;
class CFindComponentVisitor;
class CDumpComponentTreeVisitor;

//////////////////////////////////////////////////////////////////////////////
//! Building block for the trees used in the application.
/*!
    The CComponent class is the basic building block used in the various
    trees that can be found in this application.

    The CComponent class provides methods for access to parents and children,
    properties, name/type methods and visitors.

    \par Parents and children
    Some notes regarding parents and children:
    <ul>
    <li> A component can have multiple children. The order in which the
         children are added is preserved. </li>
    <li> A component can have only one parent. If a component is added as a
         child to a new parent component, it is automatically removed from the
         list of children of the old parent.</li>
    <li> A component's children can be accessed by their name or by their
         position in the list of children.</li>
    </ul>

    \par Properties
    The CComponent class provides methods that can be used to store and retrieve
    properties. A property is a mapping between a descriptive key and a vector
    of values associated with this key. For example: we could set the property
    <em>telephone nrs</em> to the values <em>010-1234567, 06-12345678</em>.

    The method provided allow easy access to the vector of values or just the
    first value of the vector. This means the properties can be seen as either
    a 1-1 mapping or a 1-n mapping, depending on what is needed.

    The values and keys are stored as <em>strings</em>. One could argue
    this class should be made a template class, with the template parameters
    the types used for the keys and values. I disagree however. This would still
    mean that ALL properties have these types. The <em>string</em> type is most
    flexible, because it is relatively easy to convert values to strings and
    vice versa. To make it easier to store multiple values for one property,
    the vector methods are provided.

    In this application, the property keys used are all defined in Keywords.h.

    \par Component name and type
    A component can have a name and type. These are strings so it is possible
    to use anything. However, names cannot contain dots. See below for the reason.
    In this application the names are identifiers and the types
    are specified in Keywords.h.

    Since components can be used to form trees, it is also possible to get the
    <em>full-context name</em>. The full context name is build from all the
    parents names and this components name, seperated by dots.
    For example, if C is a child of B and B is a child of A then the full-context
    name for C would be A.B.C .

    \par Visitors
    To make it possible to "walk the tree", accept() methods are provided.
    Currently, the accept() accepts ComponentVisitor objects and CDumpComponentTreeVisitor
    objects. accept() does a depth-first traversal of the tree, but an accept method
    that does breadth-first traversal could easily be added
    (For those wondering how this all works, this is the Visitor pattern as described
     by Erich Gamma and others in their wonderfull book `Design Patterns'.).

    \par Components in this application
    In this application, Components represent elements of a user interface.
    Possible elements can be tabpages, dropdowns, comboboxes, spreadsheet,
    strings, numbers, etc. But also some more specific user interface elements
    are possible, like the custom conditionlist dialog and colors.

    For a complete language reference, please take a look at Xander Burgerhout's
    graduation report or the Techfile language reference.

    \sa CComponentTree, CGUITree, CGeneratorComp, CGenerator, CComponentVisitor,
        CFindComponentVisitor

    \author Xander Burgerhout
*/
class CComponent
{
    private:
	string              m_identifier;   //!< The name of this component
	string              m_type;         //!< The type of this component

	CComponent*         m_parent;       //!< The parent of this component
	vector<CComponent*> m_children;     //!< The children of this component
	map<string, vector<string> > m_properties; //!< The properties of this component

    public:
	CComponent(CComponent* parent = 0, const string& name = "Unnamed");
	virtual ~CComponent();

	// Identification methods
	virtual void            setName(const string& name);
	virtual string          getName() const;
	virtual string          getFullContextName() const;
	virtual void            setType(const string& name);
	virtual string          getType() const;

	// Family methods
	virtual CComponent*     getParent() const;
	virtual void            add(CComponent* component);
	virtual CComponent*     getChild(int index) const;
	virtual CComponent*     getChild(const string& name) const;
	virtual int             numChildren();
	virtual int             childPos(const string& name);
	virtual void            removeChild(CComponent* child);

	// Operations
	virtual string          getProperty(const string& prop);
	virtual bool		getProp(const string& prop, string& value);
	virtual void            setProperty(const string& prop, const string& value);
	virtual void            addProperty(const string& prop, const string& value);
	virtual vector<string>  getPropertyVector(const string& prop);
	virtual void            setPropertyVector(const string& prop, const vector<string>& values);

	CComponent&             operator=(CComponent& src);

	// Debug methods
	virtual void            dump();

	// Visitors
	virtual void            accept(CComponentVisitor&);
	virtual void            accept(CDumpComponentTreeVisitor&);
};

#endif // __COMPONENT_H__
