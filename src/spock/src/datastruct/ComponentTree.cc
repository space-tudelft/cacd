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
#include "src/spock/src/datastruct/Component.h"
#include "src/spock/src/datastruct/ComponentTree.h"
#include "src/spock/src/datastruct/Visitors.h"
#include "src/spock/src/datastruct/Keywords.h"

using namespace std;

//============================================================================
//! Default constructor.
/*!
    The default root type is KEY_TABPAGE. This means all root elements
    of this tree must be tabpages.
*/
CComponentTree::CComponentTree()
{
    m_rootType = KEY_TABPAGE;
}

//============================================================================
//! Constructor that can set the root type.
/*! This constructor sets the root type to \a rootType.
    All root components must be of this type.

    If \a rootType is an empty string, all types are allowed.

    \param rootType     type of the root components
 */
CComponentTree::CComponentTree(const string& rootType)
{
    m_rootType = rootType;
}

//============================================================================
//! Adds a root component
/*! If the component's type is not the same as the required root type,
    nothing happens. Otherwise, it is added to the tree if it was not already
    present. This means each component is only once present in the component
    tree.

    \param comp     The component to add
 */
void CComponentTree::addRootComponent(CComponent* comp)
{
    if (comp != 0 && (m_rootType == "" || comp->getType() == m_rootType)) {
        bool found = false;
        for (unsigned int i=0; i<m_components.size() && found==false; ++i) {
            if (m_components[i] == comp)
                found = true;
        }
        if (found == false)
            m_components.push_back(comp);
    }
}

//============================================================================
//! Creates a copy of a branch of the component tree.
/*!
    For each component in the branch, a new component is created with <em>new</em>
    and the properties and other values are copied into it. This is also done
    for all of the children. These children are then added to the newly created
    component, resulting in an exact copy that occupies new memory.

    \param comp     The component that specifies the root of the branch to copy.
    \return A copy of the requested branch

    \sa copyComponentTree()
 */
CComponent* CComponentTree::copyComponentBranch(CComponent* comp)
{
    CComponent* result = new CComponent;

    *result = *comp;
    for (int i=0; i<comp->numChildren(); ++i) {
        CComponent* newChild = copyComponentBranch(comp->getChild(i));
        result->add(newChild);
    }
    return result;
}

//============================================================================
//! Creates a copy of the entire component tree.
/*!
    Does the same as copyComponentBranch() only this time for all the root
    components in this tree, effectively creating a copy of this component tree.

    \return A copy of this CComponentTree.

    \sa copyComponentBranch()
 */
CComponentTree* CComponentTree::copyComponentTree()
{
    CComponentTree* copy = new CComponentTree();
    copy->m_rootType = m_rootType;

    for (unsigned int i=0; i<m_components.size(); ++i)
        copy->m_components.push_back(copyComponentBranch(m_components[i]));
    return copy;
}

//============================================================================
//! Finds a component with name \a identifier.
/*!
    If multiple components are found, the first hit is returned. Use search()
    if all the results are needed.

    \param identifier   The name of the component to find.
    \return A pointer to the component found or 0 if it could not be found.

    \sa search(), CFindComponentVisitor
*/
CComponent* CComponentTree::findComponent(const string& identifier)
{
//    debug("CComponentTree::findComponent(%s)", identifier.c_str());
    CFindComponentVisitor find = search(identifier);
//    debug("Done search for %s in %p, hits = %d", identifier.c_str(), this, find.hitCount());

    if (find.hitCount() != 0)
        return find.getHit();
    return 0;
}

//============================================================================
//! Finds all occurances of \a name in the component tree.
/*! A search is performed with CFindComponentVisitor. The results of this
    search are then returned.

    \param name     Name of the component to find.
    \return A CFindComponentVisitor with the search results.
    \sa CFindComponentVisitor, findComponent()
*/
CFindComponentVisitor CComponentTree::search(const string& name)
{
    CFindComponentVisitor find(name);

    for (unsigned int i=0; i<m_components.size(); ++i)
        m_components[i]->accept(find);
    return find;
}

//============================================================================
//! Returns the root components of this tree.
/*!
    \return A vector with the root components of this tree.
    \sa addRootComponent(), getRootType()
 */
vector<CComponent*> CComponentTree::getRootComponents()
{
    vector<CComponent*> result;

    for (unsigned int i=0; i<m_components.size(); ++i) {
        if (m_components[i]->getType() == KEY_TABPAGE)
            result.push_back(m_components[i]);
    }
    return result;
}

//============================================================================
//! Returns the type of components that are accepted as roots.
/*!\return The type of components that are accepted as roots.
   \sa getRootComponents(), addRootComponent()
 */
string CComponentTree::getRootType()
{
    return m_rootType;
}

//============================================================================
//! Does an accept() for all the root components
/*!
    This method passes on the visitor to the root components present in this
    tree. CComponent::accept() is called for each root component.

    \param visitor      The visitor to accept.
    \sa CComponent, CComponent::accept(), CFindComponentVisitor
 */
void CComponentTree::accept(CComponentVisitor& visitor)
{
    for (unsigned int i=0; i<m_components.size(); ++i)
        m_components[i]->accept(visitor);
}

//============================================================================
//! Dumps debug information
void CComponentTree::dump()
{
    for (unsigned int i=0; i<m_components.size(); ++i) {
        CDumpComponentTreeVisitor dumper;
        m_components[i]->accept(dumper);
    }
}
