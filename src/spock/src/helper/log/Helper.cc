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

#include "src/spock/src/helper/log/Helper.h"

using namespace std;

//============================================================================
//! Removes whitespace
/*!
    \author Arjen van Rhijn
 */
void RemoveSpaces(string& str)
{
    string tmp;
    tmp.resize(str.size());

    string::iterator src = str.begin();
    string::iterator dest = tmp.begin();

    bool sp = false;
    bool lp = false;

    while (*src == ' ' || *src == '\t') src++;

    while (src != str.end())
    {
	if (*src == '\t') sp = true;
	else if (*src == ' ') sp = true;
	else if (sp) {
		lp = true;
		sp = false;
	}

	if (lp)	 *dest++ = ' ';
	if (!sp) *dest++ = *src;

	lp = false;
	src++;
    }

    *dest = '\0';
    str = tmp;
}

//============================================================================
//! Splits a string.
/*!
  \param src        The string that must be split
  \param splitChar  The character or string that defines the split points

  \return
    A vector containing the split parts of the string.

    If the \a splitChar sequence was not found, a vector with only one element
    is returned. The element contains the \a src string.

    If the \a splitChar sequence was found, the vector is filled with the
    string parts.

    Examples:
    if "a,b,c,d" is to be split on the "," character the result will be
    a vector with four strings, "a" "b" "c" and "d".

    If the string is to be split on "c," the result will be a vector
    with two strings, "a,b," and "d".
 */
vector<string> splitString(const string& src, const string& splitChar /* = " "*/)
{
    vector<string>  result;
    string source = src;

    string::size_type idx;

    idx = source.find(splitChar);
    while (idx != string::npos) {
        result.push_back(source.substr(0, idx));
        source = source.substr(idx+splitChar.length());
        idx = source.find(splitChar);
    }
    if (idx == string::npos)
        result.push_back(source);
    return result;
}

//============================================================================
//! Joins strings
/*!
    \param stringVec    The source strings
    \param joinChar     The character or string to use for joining.

    \return
    A string containing the complete joined string.

    Example:
    If \a stringVec is "a" "b" "c" and "d" and the \a joinChar sequence is "->",
    the result will be "a->b->c->d".
 */
string joinStrings(const vector<string>& stringVec, const string& joinChar /* = " " */)
{
    string result = "";

    for (unsigned int i=0; i<stringVec.size(); ++i)
        result += stringVec[i] + joinChar;

    if (stringVec.size() > 0)
        result.resize(result.size() - joinChar.size());

    return result;
}
