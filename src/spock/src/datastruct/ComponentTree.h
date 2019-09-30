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
#ifndef __COMPONENTTREE_H__
#define __COMPONENTTREE_H__

// STL includes
#include <vector>
#include <string>

// Forward declarations
class CComponent;
class CComponentVisitor;
class CFindComponentVisitor;

//============================================================================
//! Binds together CComponent objects
/*! The CComponentTree class can bind together CComponent objects.

    It is possible to inherit from CComponent and overload some members.
    However, because the methods provided by this class and the real purpose
    of this class are quite different from the CComponent class, this has not
    been done.

    The difference is that this class is specifically provided to perform
    actions on the entire tree, like searching. It also allows the programmer
    to create a true copy of the components. Finally, it is possible to allow
    just one type of components to become root components, for example tabpages.

    Again, this could also be done by subclassing CComponent. However, I think
    this would not clarify the structure of the application and the purpose of
    this class. Not subclassing CComponent also prevents strange behaviour of the
    CComponent::getFullContextName() method, which would have to be adapted
    to ignore this type of component.

    The CComponentTree class provides methods for:
    <ul>
        <li> Adding root components </li>
        <li> Copying of trees and branches </li>
        <li> Finding single or multiple occurances of a component with a
             certain name </li>
        <li> An accept() method that will let the visitor visit the entire
             tree </li>
    </ul>

    \sa CComponent, CComponentVisitor, CFindComponentVisitor
    \author Xander Burgerhout
*/
class CComponentTree
{
    private:
	vector<CComponent*>     m_components;   //!< The root components
	string                  m_rootType;     //!< The type of components allowed

    public:
	CComponentTree();
	explicit CComponentTree(const string& rootType);

	void                    addRootComponent(CComponent* comp);
	CComponent*             copyComponentBranch(CComponent* comp);
	CComponent*             findComponent(const string& name);
	CFindComponentVisitor   search(const string& name);
	CComponentTree*         copyComponentTree();
	vector<CComponent*>     getRootComponents();
	string                  getRootType();

	void                    accept(CComponentVisitor&);

	void                    dump();
};

#endif
