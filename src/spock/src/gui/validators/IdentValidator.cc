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
#include "src/spock/src/gui/validators/IdentValidator.h"

// Qt includes
#include <qregexp.h>
#include <qwidget.h>

#include <stdlib.h>

using namespace std;

//============================================================================
//! Constructor
CIdentValidator::CIdentValidator(QWidget* parent, const char* name)
    : QValidator(parent, name)
{
}

CIntValidator::CIntValidator(QWidget* parent, const vector<string>& values)
    : QValidator(parent, 0)
{
	m_min = -2147483647;
	m_max =  2147483647;
	if (values.size() > 0) {
	    if (values[0] != "") m_min = atof(values[0].c_str());
	    if (values[1] != "") m_max = atof(values[1].c_str());
	}
}

CDoubleValidator::CDoubleValidator(QWidget* parent, const vector<string>& values)
    : QValidator(parent, 0)
{
	m_min = -1e100;
	m_max =  1e100;
	if (values.size() > 0) {
	    if (values[0] != "") m_min = atof(values[0].c_str());
	    if (values[1] != "") m_max = atof(values[1].c_str());
	}
}

//============================================================================
//! Validation of the input.
/*!
    \param input    The input to validate
    \param pos      Position of the cursor. Can be changed.

    The input is validated using the regular expression ^[a-zA-Z_0-9]*$

    \return The validator returns QValidator::Acceptable if the input is
    considered an identifier. QValidator::Invalid is returned otherwise.
 */
QValidator::State CIdentValidator::validate(QString& input, int& pos) const
{
    if (input.length() == 0) return QValidator::Acceptable;
    QRegExp expr("^[a-zA-Z_][a-zA-Z_0-9]*$");
    int res = expr.search(input, 0);
    return (res == 0)? QValidator::Acceptable : QValidator::Invalid;
}

QValidator::State CIntValidator::validate(QString& input, int& pos) const
{
    const char *s;
    if (input.length() == 0) return QValidator::Acceptable;
    if (m_min >= 0) s = "^[0-9]*$";
    else s = "^-?[0-9]*$";
    QRegExp expr(s);
    if (expr.search(input, 0) != 0) return QValidator::Invalid;
    double v = atof(input.latin1());
    if (v < m_min || v > m_max) return QValidator::Invalid;
    return QValidator::Acceptable;
}

QValidator::State CDoubleValidator::validate(QString& input, int& pos) const
{
    const char *s;
    int res;
    int len = input.length();
    if (len == 0) return QValidator::Acceptable;

    if (m_min >= 0) s = "^[0-9]*\\.?[0-9]*$";
    else s = "^-?[0-9]*\\.?[0-9]*$";
    if ((res = input.find('e')) >= 0) {
	if (res == 0) return QValidator::Invalid;
	QRegExp expr(s);
	if (expr.search(input.left(res), 0) != 0) return QValidator::Invalid;
    }
    else {
	QRegExp expr(s);
	if (expr.search(input, 0) != 0) return QValidator::Invalid;
    }
    if (res >= 0) {
	int n = input.find('-');
	if (res == 1 && (n == 0 || input.find('.') == 0)) return QValidator::Invalid;
	if (res == 2 && (n == 0 && input.find('.') == 1)) return QValidator::Invalid;
	++res;
	if (n >= 0) {
	    if (n < res) n = input.find('-', res);
	    if (n > 0) {
		if (n != res) return QValidator::Invalid;
		++res;
	    }
	}
	if (len > res) {
	    if (len > res+2) return QValidator::Invalid;
	    if (len == res+1) {
		QRegExp expr("[1-9]");
		if (expr.search(input, res) != res) return QValidator::Invalid;
	    }
	    else {
		QRegExp expr("[0-9]");
		++res;
		if (expr.search(input, res) != res) return QValidator::Invalid;
	    }
	}
    }
    double v = atof(input.latin1());
    if (v < m_min || v > m_max) return QValidator::Invalid;
    return QValidator::Acceptable;
}
