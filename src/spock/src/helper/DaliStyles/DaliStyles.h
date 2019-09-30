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
#ifndef __DALISTYLES_H__
#define __DALISTYLES_H__
//============================================================================
//! Contains the CDaliColorTable, CDaliStyle and CDaliPixmap classes.
/*! \file

    CDaliColorTable is a singleton class that provides a translation
    between the dali color numbers and the corresponding RGB colors.

    CDaliStyle combines a dali fill style with a dali color number.

    CDaliPixmap is QPixmap derived class. CDaliPixmap contains a CDaliStyle.

    \author Xander Burgerhout
    \date Beginning of June, 2000       First versions.
*/
//============================================================================
// Qt includes
#include <qpixmap.h>

// STL includes
#include <string>
#include <vector>

using namespace std;

//! Provides conversion between dali color numbers and rgb color numbers.
/*!
    The CDaliColorTable class implements a conversion between dali color
    numbers and their corresponding rgb colors.

    This class is implemented as a singleton. A simple global array could
    also have been implemented, but the singleton implementation has some
    advantages:
        <ul>
        <li> Using a class eliminates the namespace polution of globals.
        <li> The GetColorName() function performs out of bounds checking
             on the requested number, this adds safety.
        <li> The class is just as accesible as a global, because it is
             implemented as a singleton.
        <li> Because the class encapsulates behaviour, any future changes
             to the dali color mechanism can be shielded from the rest of
             the application.
        </ul>

    \author Xander Burgerhout
*/
class CDaliColorTable
{
    private:
	vector<string>                  m_colorTable;   //!< The dali to rgb color table
	static CDaliColorTable*         m_singleton;    //!< Makes this class singleton

    protected:
	CDaliColorTable();

    public:
	~CDaliColorTable();

	static CDaliColorTable*  Instance();

	const char*              GetColorName(int index);
	int                      NumColors();
};

//! CDaliStyle combines a dali fill style with a dali color number.
/*!
    Combines dali color and fill to create a dali style.

    \author Xander Burgerhout
*/
class CDaliStyle
{
    public:
    // Change these numbers to alter display order.
    enum DaliFill {
	Hashed      = 0,
	Solid       = 1,
	Hollow      = 2,
	Hash12Out   = 3,
	Hash25Out   = 4,
	Hash50Out   = 5,
	Hash12      = 6,
	Hash25      = 7,
	Hash50      = 8,
	NumFills    = 9
    };

    private:
	int             m_fill;
	int             m_color;

    protected:

    public:
	CDaliStyle();
	CDaliStyle(int color, int fill);
	CDaliStyle(const CDaliStyle& src);

	const char*     GetColorName();
	int             GetColor();
	void            SetColor(int newColor);
	int             GetFill();
	void            SetFill(int newFill);

	CDaliStyle&     operator=(const CDaliStyle& src);
};

//! CDaliPixmap is QPixmap derived class. CDaliPixmap contains a CDaliStyle.
/*!
    CDaliPixmap is nothing more than a QPixmap with a CDaliStyle member and
    methods to get the color number/name and fill.

    As a bonus, a dali style generator is incorporated. See CreateDaliPixmap().

    \author Xander Burgerhout
*/
class CDaliPixmap : public QPixmap
{
    private:
	CDaliStyle                      m_daliStyle;

    public:
	CDaliPixmap();
	CDaliPixmap(int width, int height, int color, int fill);

	const char*                     GetColorName();
	int                             GetColor();
	void                            SetColor(const int& newColor);
	int                             GetFill();
	void                            SetFill(const int& newFill);

	void                            CreateDaliPixmap(int width, int height);
};

#endif // __DALISTYLES_H__
