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
#ifndef __IDENTVALIDATOR_H__
#define __IDENTVALIDATOR_H__

// Qt includes
#include <qvalidator.h>

// STL includes
#include <vector>
#include <string>

using namespace std;

// Forward declarations
class QWidget;

//! Validates the input of identifiers
/*!
    This QValidator derived class validates the given input to
    see if is a valid identifier.

    An identifier is consists of the characters a-z,A-Z,0-9 and
    the underscore _. Leading whitespace is ignored.

    \author Xander Burgerhout
*/
class CIdentValidator : public QValidator
{
    public:
	CIdentValidator(QWidget* parent, const char* name = 0);

	State           validate(QString& input, int& pos) const;
};

class CIntValidator : public QValidator
{
    private:
	double		m_min, m_max;

    public:
	CIntValidator(QWidget* parent, const vector<string>& values);

	State           validate(QString& input, int& pos) const;
};

class CDoubleValidator : public QValidator
{
    private:
	double		m_min, m_max;

    public:
	CDoubleValidator(QWidget* parent, const vector<string>& values);

	State           validate(QString& input, int& pos) const;
};

#endif // __IDENTVALIDATOR_H__
