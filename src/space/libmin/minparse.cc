/*
 * ISC License
 *
 * Copyright (C) 1997-2018 by
 *	Arjan van Genderen
 *	Kees-Jan van der Kolk
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include "src/space/libmin/minrobdd.h"
#include "src/space/libmin/minparse.h"

typedef char* charp;

#define skip_whitespace() \
    while(*it == ' ' || *it == '\t' || *it == '\n' || *it == '\r') ++it

static void error(const string& what)
{
    cerr << "minimize: error: " << what << endl;
    die();
}

//==============================================================================
//
//  Parsing routines (recursive-descend style).
//
//==============================================================================

static STDDWord read_expression(MINRobdd& robdd, charp& it);

static STDDWord read_primary(MINRobdd& robdd, charp& it)
{
    skip_whitespace();
    if(!*it) error("Expected a primary expression.");

    if(*it == '!')
    {
        ++it;
        STDDWord f = read_primary(robdd, it);
        return robdd.ite(f, 0, 1);
    }

    if(*it == '(')
    {
        ++it;
        STDDWord f = read_expression(robdd, it);
        skip_whitespace();
	if(*it != ')') error("Expected `)'.");
        ++it;
        return f;
    }

    if(*it == '0') { ++it; return 0; }

    if(*it == '1') { ++it; return 1; }

    /* read_identifier */

    skip_whitespace();

    char name[40];
    int i = 0;

    if(*it == '-' || *it == '=') name[i++] = *it++;

    if(!((*it >= 'A' && *it <= 'Z') ||
         (*it >= 'a' && *it <= 'z') ||
         (*it == '_'))) error("Expected an identifier.");

    name[i++] = *it++;

    while((*it >= 'A' && *it <= 'Z') ||
          (*it >= 'a' && *it <= 'z') ||
          (*it >= '0' && *it <= '9') ||
          (*it == '_'))
    {
	if (i > 34) error("Too long identifier.");
	name[i++] = *it++;
    }
    name[i] = 0;

    return robdd.symbolNode(name);
}

static STDDWord read_term(MINRobdd& robdd, charp& it)
{
    STDDWord left = read_primary(robdd, it);

    for(;;)
    {
        skip_whitespace();

        if(!((*it >= 'A' && *it <= 'Z') ||
             (*it >= 'a' && *it <= 'z') ||
             (*it == '_') ||
             (*it == '-') ||   // <- Note: identifier may also start with `-' or `='.
             (*it == '=') ||
             (*it == '0') ||
             (*it == '1') ||
             (*it == '!') ||
             (*it == '('))) break;

        STDDWord right = read_primary(robdd, it);

        left = robdd.ite(left, right, 0); // <- Meaning: if "left" then "right" else "0";
    }

    return left;
}

static STDDWord read_expression(MINRobdd& robdd, charp& it)
{
    STDDWord left = read_term(robdd, it);

    for(;;)
    {
        skip_whitespace();
        if(*it != '|') break;
        ++it;

        STDDWord right = read_term(robdd, it);

        left = robdd.ite(left, 1, right); // <- Meaning: if "left" then "1" else "right";
    }

    return left;
}

STDDWord min_parse_expression(MINRobdd& robdd, const char* expression)
{
    charp it = (charp) expression;
    STDDWord f = read_expression(robdd, it);

    skip_whitespace();
    if(*it) error("Unexpected trailing characters.");

    return f;
}
