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

//============================================================================
//! Contains the CDaliColorTable, CDaliStyle and CDaliPixmap classes.
/*! \file

    CDaliColorTable is a singleton class that provides a translation
    between the dali color numbers and the corresponding RGB colors.

    CDaliStyle combines a dali fill style with a dali color number.

    CDaliPixmap is QPixmap derived class. CDaliPixmap contains a CDaliStyle.

    \author Xander Burgerhout
    \par Remarks
         For a list of changes and updates, please refer to the header file
         (DaliStyles.h).
*/
//============================================================================

// Project includes
#include "src/spock/src/helper/DaliStyles/DaliStyles.h"

// Qt includes
#include <qpainter.h>

CDaliColorTable* CDaliColorTable::m_singleton = 0;

using namespace std;

//============================================================================
/*! CDaliColorTable constructor.

    The constructor makes the Dali color conversion table.

    Since the maskdata specifies the colors as a number from 0 to 7 instead
    of in a RGB format, order is important. Do NOT change the order of the
    colornames in this contructor unless the color specification for Dali has
    changed.
*/
CDaliColorTable::CDaliColorTable()
{
    // Do NOT change order here!
    // This order defines the colors the way Dali does!
    // Any changes will result in the wrong Dali color numbers in the
    // generated maskdata file!
    m_colorTable.resize(8);
    m_colorTable[0] = "black";
    m_colorTable[1] = "red";
    m_colorTable[2] = "green";
    m_colorTable[3] = "yellow";
    m_colorTable[4] = "blue";
    m_colorTable[5] = "violet";
    m_colorTable[6] = "cyan";
    m_colorTable[7] = "white";
};

//============================================================================
/*!
    \return The only single instance of the colortable. If the colortable has
    not yet been created, it is created now.
*/
CDaliColorTable* CDaliColorTable::Instance()
{
    if (m_singleton == 0) {
        m_singleton = new CDaliColorTable;
    }
    return m_singleton;
}

//============================================================================
/*!
    \param  index           The dali color number
    \return The RGB color name. Returns white if index was out of bounds.
*/
const char* CDaliColorTable::GetColorName(int index)
{
    Instance(); // Make sure the constructor was called

    if (index>=0 && (unsigned)index<m_colorTable.size())
        return m_colorTable[index].c_str();

    return "white";
}

//============================================================================
/*!
    \return The number of colors in the color table.
*/
int CDaliColorTable::NumColors()
{
    return m_colorTable.size();
}

//============================================================================
/*! Constructor */
CDaliStyle::CDaliStyle()
{
    m_fill = CDaliStyle::Solid;
    m_color = 7;    // black;
}

//============================================================================
/*! Contructs a CDaliStyle object with \a color and \a fill.

    \param  yourparam   yourdesc
    \return yourdesc
*/
CDaliStyle::CDaliStyle(int color, int fill)
{
    m_fill = fill;
    m_color = color;
}

CDaliStyle::CDaliStyle(const CDaliStyle& src)
{
    *this = src;
}

CDaliStyle& CDaliStyle::operator=(const CDaliStyle& src)
{
    m_fill = src.m_fill;
    m_color = src.m_color;
    return *this;
}

//============================================================================
/*!
    \return The RGB color name of this style.
*/
const char* CDaliStyle::GetColorName()
{
    return CDaliColorTable::Instance()->GetColorName(m_color);
}

//============================================================================
/*! \return The dali color number of this style. */
int CDaliStyle::GetColor()
{
    return m_color;
}

//============================================================================
/*! Sets the style's color to \a newColor. */
void CDaliStyle::SetColor(int newColor)
{
    m_color = newColor;
}

//============================================================================
/*! \return The dali fill number of this style. */
int CDaliStyle::GetFill()
{
    return m_fill;
}

//============================================================================
/*! Sets the style's fill to \a newFill. */
void CDaliStyle::SetFill(int newFill)
{
    m_fill = newFill;
}

//============================================================================
/*! The default constructor. */
CDaliPixmap::CDaliPixmap() : QPixmap()
{
    m_daliStyle.SetColor(7);    // white
    m_daliStyle.SetFill(CDaliStyle::Solid);
}

//============================================================================
/*! This contructor creates the pixmap from the given parameters.

    \param  width   Width of the pixmap
    \paran  height  Height of the pixmap
    \param  color   Dali color number
    \param  fill    Dali fill number
*/
CDaliPixmap::CDaliPixmap(int width, int height, int color, int fill) : QPixmap()
{
    m_daliStyle.SetColor(color);
    m_daliStyle.SetFill(fill);
    CreateDaliPixmap(width, height);
}

//============================================================================
/*! \return The RGB color name.
*/
const char* CDaliPixmap::GetColorName()
{
    return m_daliStyle.GetColorName();
}

//============================================================================
/*! \return The dali color number of this style. */
int CDaliPixmap::GetColor()
{
    return m_daliStyle.GetColor();
}

//============================================================================
/*! Sets the style's color to \a newColor and redraws the pixmap. */
void CDaliPixmap::SetColor(const int& newColor)
{
    if (newColor!=m_daliStyle.GetColor()) {
        m_daliStyle.SetColor(newColor);
        CreateDaliPixmap(width(), height());
    }
}

//============================================================================
/*! \return The dali fill number of this style. */
int CDaliPixmap::GetFill()
{
    return m_daliStyle.GetFill();
}

//============================================================================
/*! Sets the style's fill to \a newFill and redraws the pixmap. */
void CDaliPixmap::SetFill(const int& newFill)
{
    if (newFill!=m_daliStyle.GetFill()) {
        m_daliStyle.SetFill(newFill);
        CreateDaliPixmap(width(), height());
    }
}

//============================================================================
/*!
    Draws this pixmap according to the dali color number and fill stored in
    the object, with the specified width and height.

    The created pixmaps approximate the drawing style in dali as good as
    possible.
    \param newWidth     Width of the new pixmap.
    \param newHeight    Height of the new pixmap
*/
void CDaliPixmap::CreateDaliPixmap(int newWidth, int newHeight)
{
    resize(newWidth-1, newHeight-1);
    fill();

    if (!isNull()) {

        QPainter paint(this);
        paint.setClipRect(0, 0, newWidth-1, newHeight-1);
        paint.setClipping(true);
        QColor   color(m_daliStyle.GetColorName());
        QBrush   brush;
        QPen     outlinePen(color, 1);

        paint.fillRect(0, 0, newWidth, newHeight, Qt::black); // black background
        paint.setPen(m_daliStyle.GetColorName());

        int stepSize = 0;
        bool wantOutline = false;

        switch (m_daliStyle.GetFill()) {
            case CDaliStyle::Solid:
                paint.fillRect(0, 0, newWidth, newHeight, color);
                break;
            case CDaliStyle::Hashed:
                stepSize = 8;
                break;
            case CDaliStyle::Hollow:
                wantOutline = true;
                break;
            case CDaliStyle::Hash12Out:
                wantOutline = true;
            case CDaliStyle::Hash12:
                stepSize = 8;
                break;
            case CDaliStyle::Hash25Out:
                wantOutline = true;
            case CDaliStyle::Hash25:
                stepSize = 4;
                break;
            case CDaliStyle::Hash50Out:
                wantOutline = true;
            case CDaliStyle::Hash50:
                stepSize = 2;
                break;
        }

        int fill = m_daliStyle.GetFill();
        if ( fill!=CDaliStyle::Solid && fill!=CDaliStyle::Hollow ) {
            for (int i=-newHeight; i<newWidth-1; i+=stepSize)     // Qt clips for us.
                paint.drawLine(i, 0, i+newHeight-1, newHeight-1);
        }

        if (wantOutline) {
            paint.setPen(outlinePen);
            paint.drawRect(0, 0, newWidth-1, newHeight-1);
        }
    }
}
