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
#include "src/spock/src/datastruct/Visitors.h"
#include "src/spock/src/helper/log/Helper.h"

// Qt includes
#include <qwidget.h>
#include <qlayout.h>

using namespace std;

//============================================================================
//! Default constructor
/*! \a parent will be this components parent. If the parent is not 0, this
    component is immediately added as a child to that parent.

    \a name is an optional name. If no name is specified, the default "Unnamed"
    will be assumed.

    \param parent   The parent component. Defaults to 0.
*/
CComponent::CComponent(CComponent* parent /* = 0 */, const string& name /*= "Unnamed"*/)
{
    m_parent = 0;

    if (parent != 0) parent->add(this);
    m_parent = parent;
    m_identifier = name;
}

//============================================================================
//! Default destructor
CComponent::~CComponent()
{
    for (int i=0; i<numChildren(); ++i)
        delete m_children[i];

    m_properties.clear();
    m_parent = 0;
}

//============================================================================
//! Sets the name of this component
/* \param name     The new name.
   \sa getName(), getFullContextName()
 */
void CComponent::setName(const string& name)
{
    m_identifier = name;
}

//============================================================================
//! Returns the name of this component
/*! \return The name of the current component without its context
    \sa getFullContextName()
 */
string CComponent::getName() const
{
    return m_identifier;
}

//============================================================================
//! Returns the name of this component including the context
/*! \return The name of the current component including the full context,
            e.g. 'page1.params.resistance'
 */
string CComponent::getFullContextName() const
{
    CComponent* parent = getParent();
    string name = "";
    if (parent != 0)
        name = parent->getFullContextName() + "." + m_identifier;
    else
        name = m_identifier;
    return name;
}

//============================================================================
//! Sets the component's type.
/*! In principal, the type can be any valid string. For a list of the current
    types see the class description. A list of all Keywords can be found in
    Keywords.h.

    \param type     The new type.
    \sa getType(), Keywords.h
 */
void CComponent::setType(const string& type)
{
    m_type = type;
}

//============================================================================
//! Returns the type of this component.
/*! \sa getType()
    \return The type of this component.
 */
string CComponent::getType() const
{
    return m_type;
}

//============================================================================
//! Returns the value of property \a prop.
/*!
    \returns the first value in the property list of property \a prop.
    \param prop     Name of the property.
    \sa getPropertyVector(), setPropertyVector(), setProperty().
 */
string CComponent::getProperty(const string& prop)
{
    vector<string> vals = getPropertyVector(prop);
    if (vals.size() > 0) return vals[0];
    return "";
}

bool CComponent::getProp(const string& prop, string& value)
{
    vector<string> vals = getPropertyVector(prop);
    if (vals.size() > 0) {
	value = vals[0];
	return true;
    }
    return false;
}

//============================================================================
//! Set a property of this component
/*!
    The value vector of property \a prop will be cleared and \a value will be
    set as the only property value.

    \param  prop    property to set
    \param  value   value to set the property to
*/
void CComponent::setProperty(const string& prop, const string& value)
{
    m_properties[prop].clear();
    m_properties[prop].push_back(value);
}

//============================================================================
//! adds \a value to the list of values for property \prop.
/*! The new value will be appended.

    \param  prop    property to set
    \param  value   value to set the property to
    \sa setProperty(), getProperty()
*/
void CComponent::addProperty(const string& prop, const string& value)
{
    m_properties[prop].push_back(value);
}

//============================================================================
//! Returns the list of values for property \a prop.
/*! \return Returns the list of values for property \a prop.

    If the list is empty an empty vector<string> will be returned.

    \param  prop    property to retrieve values of
    \sa getProperty(), setProperty(), setPropertyVector().
*/
vector<string> CComponent::getPropertyVector(const string& prop)
{
    if (m_properties.count(prop) > 0)
        return m_properties[prop];

    vector<string> empty;
    return empty;
}

//============================================================================
//! Sets the list of values for property \prop.
/*! Any old property values will be removed.

    \param  prop    property to set
    \param  values  vector with values to set
    \sa setProperty(), getProperty(), getPropertyVector().
*/
void CComponent::setPropertyVector(const string& prop, const vector<string>& values)
{
    m_properties[prop] = values;
}

//============================================================================
//! Returns this component's parent.
/*! \return The parent or a 0 if there is no parent.

    \sa getChild()
*/
CComponent* CComponent::getParent() const
{
    return m_parent;
}

//============================================================================
//! Adds a child to this component.
/*!
    If the component was a child of another compononent, that component will
    no longer be a parent (components can have only one parent). The child is
    removed from the old parents list of children and added to this components
    list of children.

    \param  newChild    the component to add
    \sa removeChild(), getParent(), getChild(), childPos(), numChildren().
*/
void CComponent::add(CComponent* newChild)
{
    ASSERT(newChild != 0);          // Perhaps this should just issue a warning

    /*
    debug(">>> CComponent::add()");
    debug("        Adding to:");
    debug("         type = %s", getType().c_str());
    debug("         name = %s", getName().c_str());
    debug("        child:");
    debug("         type = %s", newChild->getType().c_str());
    debug("         name = %s", newChild->getName().c_str());
    */
    bool found = false;
    for (int i=0; i<numChildren(); ++i)
        if (m_children[i] == newChild)
            found = true;

    if (!found) {                   // Child only allowed to be added once...
        CComponent* parent = newChild->getParent();
        if (parent != 0)
            parent->removeChild(newChild);

        m_children.push_back(newChild);
        newChild->m_parent = this;
    }
    else
        debug("    child was already present, ignoring child...");
}

//============================================================================
//! Dumps some debugging information to stderr.
void CComponent::dump()
{
    debug("===================================================================");
    debug("%s (type %s)", getFullContextName().c_str(), getType().c_str());
    debug("===================================================================");
    if (m_parent != 0)
        debug("Parent is %s", m_parent->getName().c_str());
    else
        debug("No parent.");
    debug("-------------------------------------------------------------------");
    debug("Properties set:");

    map<string, vector<string> >::iterator pos;
    for (pos = m_properties.begin(); pos != m_properties.end(); ++pos) {
        string all = joinStrings(pos->second, ", ");
        debug("  %s\t\t= %s", pos->first.c_str(), all.c_str());
    }
    debug("-------------------------------------------------------------------");
    debug("Children: (%d)", numChildren());
    for (int i=0; i<numChildren(); ++i) {
        ASSERT(getChild(i) != 0);
        CComponent* child = getChild(i);
        string name = child->getFullContextName();
        string type = child->getType();
        debug("%s\t\ttype %s", name.c_str(), type.c_str());
    }
}

//============================================================================
//! Returns the child at position \a index.
/*!
    \return The child at \a index or 0 if the index is out of bounds.
    \sa childPos(), numChildren()
*/
CComponent* CComponent::getChild(int index) const
{
    if (index >= 0 && (unsigned)index < m_children.size()) {
        ASSERT(m_children[index] != 0);
        return m_children[index];
    }
    return 0;
}

//============================================================================
//! Returns the child with name \a name.
/*! \return The child with name \a name or 0 if there is no child with that
            name. \a name is the short name, not the full context name.
 */
CComponent* CComponent::getChild(const string& name) const
{
    for (unsigned int i=0; i<m_children.size(); ++i) {
        if (m_children[i]->getName() == name)
            return m_children[i];
    }
    return 0;
}

//============================================================================
//! Returns the number of children
/*! \return The number of children
    \sa childPos(), getChild()
 */
int CComponent::numChildren()
{
    return (int)m_children.size();
}

//============================================================================
//! Returns the position of child with name \a name.
/*! \param

   \return The index at which the child with name \a name resides, or -1 if
    no child can be found.
 */
int CComponent::childPos(const string& name)
{
    for (unsigned int i=0; i<m_children.size(); ++i) {
        if (m_children[i]->getName() == name)
            return i;
    }
    return -1;
}

//============================================================================
//! Removes a child
/*! If \a child is a child of this component, it is removed from the list of
    children.

    WARNING: do not forget to set the parent of \a child to 0 or add it to
    a new parent.

    \param child    the component to remove
    \sa addChild(), getParent(), getChild(), childPos(), numChildren().
*/
void CComponent::removeChild(CComponent* child)
{
    int index = -1;
    for (unsigned int i=0; i<m_children.size(); ++i)
        if (m_children[i] == child)
            index = i;

    if (index != -1) {
        vector<CComponent*>::iterator pos = m_children.begin() + index;
        m_children.erase(pos);
    }
}

//============================================================================
//! Makes a copy of a component
/*!
    The component returned has the same name, type, and properties.
    The parent is set to 0 (null) and can be set by the
    caller if desired. The copy has no children because a component can have
    only one parent. Copying the children from \a src would violate this rule.

    \param  src     Source component
    \return A copy of \a src without any children, parent, widget or layout set.
*/
CComponent& CComponent::operator=(CComponent& src)
{
    m_identifier = src.m_identifier;
    m_type       = src.m_type;
    m_parent     = 0;
    m_properties = src.m_properties;
    return *this;
}

//============================================================================
//! Allows a CComponentVisitor to visit this component
/*
    This method is provided as part of the visitor pattern.

    The \a visitor has a member CComponentVisitor::visitComponent() wich will
    be called from this function.

    The method first calls accept() for its children and then calls the visitors
    visitComponent() method, resulting in depth-first traversal through the
    components.

    Perhaps the name of this method should be changed to acceptDepthFirst.

    \param visitor The visitor component.

    \sa CComponentVisitor::visitComponent()
*/
void CComponent::accept(CComponentVisitor& visitor)
{
    for (int i=0; i<numChildren(); ++i)
        getChild(i)->accept(visitor);

    visitor.visitComponent(this);
}

//============================================================================
//! Allows a CDumpComponentVisitor to visit this component
/*
    This method is provided as part of the visitor pattern.

    The \a visitor has a member CDumpComponentVisitor::visitComponent() wich will
    be called from this function.

    The method first calls the visitors visitComponent() method and then
    calls accept() for its children, resulting in breadth-first traversal
    through the components. However, since this component also prints
    (indented) information about the tree, it is not really suited for
    anything else then dumping debug information.

    \param visitor The visitor component.

    \sa CComponentVisitor::visitComponent()
*/
void CComponent::accept(CDumpComponentTreeVisitor& visitor)
{
    debug("%s%s {", visitor.m_leadSpace.c_str(), getName().c_str());

    string oldLead = visitor.m_leadSpace;
    visitor.m_leadSpace += "  ";
    visitor.visitComponent(this);
    for (int i=0; i<numChildren(); ++i)
        getChild(i)->accept(visitor);

    visitor.m_leadSpace = oldLead;
    debug("%s}", oldLead.c_str());
}
