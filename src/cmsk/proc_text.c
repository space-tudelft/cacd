/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	R. van der Valk
 *	S. de Graaf
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

#include "src/cmsk/extern.h"

struct charedge
{
    int x, ybottom, ytop, bodybit;
};

struct charstruct
{
    char character;
    int nrofedges;
    struct charedge edge[20];
};

struct charstruct text[39] =
{
    {'a',6,{{0,0,7,RIGHT},{1,0,2,LEFT},{1,3,6,LEFT},
	    {4,0,2,RIGHT},{4,3,6,RIGHT},{5,0,7,LEFT}}},
    {'b',8,{{0,0,1,RIGHT},{0,6,7,RIGHT},{1,1,6,RIGHT},{2,1,3,LEFT},
	    {2,4,6,LEFT},{4,1,3,RIGHT},{4,4,6,RIGHT},{5,0,7,LEFT}}},
    {'c',6,{{0,0,7,RIGHT},{1,1,6,LEFT},{4,1,2,RIGHT},
	    {4,5,6,RIGHT},{5,0,2,LEFT},{5,5,7,LEFT}}},
    {'d',6,{{0,0,1,RIGHT},{0,6,7,RIGHT},{1,1,6,RIGHT},
	    {2,1,6,LEFT},{4,1,6,RIGHT},{5,0,7,LEFT}}},
    {'e',6,{{0,0,7,RIGHT},{1,1,3,LEFT},{1,4,6,LEFT},
	    {3,3,4,LEFT},{5,0,1,LEFT},{5,6,7,LEFT}}},
    {'f',5,{{0,0,7,RIGHT},{1,0,3,LEFT},{1,4,6,LEFT},
	    {3,3,4,LEFT},{5,6,7,LEFT}}},
    {'g',7,{{0,0,7,RIGHT},{1,1,6,LEFT},{3,2,3,RIGHT},{4,1,2,RIGHT},
	    {4,5,6,RIGHT},{5,0,3,LEFT},{5,5,7,LEFT}}},
    {'h',6,{{0,0,7,RIGHT},{1,0,3,LEFT},{1,4,7,LEFT},
	    {4,0,3,RIGHT},{4,4,7,RIGHT},{5,0,7,LEFT}}},
    {'i',4,{{2,0,5,RIGHT},{2,6,7,RIGHT},{3,0,5,LEFT},{3,6,7,LEFT}}},
    {'j',6,{{1,0,2,RIGHT},{2,1,2,LEFT},{3,1,5,RIGHT},
	    {3,6,7,RIGHT},{4,0,5,LEFT},{4,6,7,LEFT}}},
    {'k',8,{{0,0,7,RIGHT},{1,0,3,LEFT},{1,4,7,LEFT},{2,4,7,RIGHT},
	    {3,4,6,LEFT},{4,0,3,RIGHT},{4,6,7,LEFT},{5,0,4,LEFT}}},
    {'l',3,{{0,0,7,RIGHT},{1,1,7,LEFT},{5,0,1,LEFT}}},
    {'m',6,{{0,0,7,RIGHT},{1,0,6,LEFT},{2,0,6,RIGHT},
	    {3,0,6,LEFT},{4,0,6,RIGHT},{5,0,7,LEFT}}},
    {'n',4,{{0,0,7,RIGHT},{1,0,6,LEFT},{4,0,6,RIGHT},{5,0,7,LEFT}}},
    {'o',4,{{0,0,7,RIGHT},{1,1,6,LEFT},{4,1,6,RIGHT},{5,0,7,LEFT}}},
    {'p',5,{{0,0,7,RIGHT},{1,0,3,LEFT},{1,4,6,LEFT},
	    {4,4,6,RIGHT},{5,3,7,LEFT}}},
    {'q',7,{{0,3,7,RIGHT},{1,4,6,LEFT},{2,0,1,RIGHT},{3,1,3,RIGHT},
	    {3,4,6,RIGHT},{4,1,7,LEFT},{5,0,1,LEFT}}},
    {'r',9,{{0,0,7,RIGHT},{1,0,3,LEFT},{1,4,6,LEFT},{3,1,3,RIGHT},
	    {4,0,1,RIGHT},{4,2,3,LEFT},{4,4,6,RIGHT},{5,0,2,LEFT},
	    {5,3,7,LEFT}}},
    {'s',6,{{0,0,1,RIGHT},{0,3,7,RIGHT},{1,4,6,LEFT},
	    {4,1,3,RIGHT},{5,0,4,LEFT},{5,6,7,LEFT}}},
    {'t',4,{{0,6,7,RIGHT},{2,0,6,RIGHT},{3,0,6,LEFT},{5,6,7,LEFT}}},
    {'u',4,{{0,0,7,RIGHT},{1,1,7,LEFT},{4,1,7,RIGHT},{5,0,7,LEFT}}},
    {'v',8,{{0,3,7,RIGHT},{1,0,3,RIGHT},{1,4,7,LEFT},{2,1,4,LEFT},
	    {3,1,4,RIGHT},{4,0,3,LEFT},{4,4,7,RIGHT},{5,3,7,LEFT}}},
    {'w',6,{{0,0,7,RIGHT},{1,1,7,LEFT},{2,1,5,RIGHT},
	    {3,1,5,LEFT},{4,1,7,RIGHT},{5,0,7,LEFT}}},
    {'x',14,{{0,0,2,RIGHT},{0,5,7,RIGHT},{1,0,1,LEFT},{1,2,5,RIGHT},
	    {1,6,7,LEFT},{2,1,3,LEFT},{2,4,6,LEFT},{3,1,3,RIGHT},
	    {3,4,6,RIGHT},{4,0,1,RIGHT},{4,2,5,LEFT},{4,6,7,RIGHT},
	    {5,0,2,LEFT},{5,5,7,LEFT}}},
    {'y',10,{{0,5,7,RIGHT},{1,3,5,RIGHT},{1,6,7,LEFT},{2,0,3,RIGHT},
	    {2,4,6,LEFT},{3,0,3,LEFT},{3,4,6,RIGHT},{4,3,5,LEFT},
	    {4,6,7,RIGHT},{5,5,7,LEFT}}},
    {'z',11,{{0,0,2,RIGHT},{0,6,7,RIGHT},{1,2,3,RIGHT},{2,1,2,LEFT},
	    {2,3,4,RIGHT},{3,2,3,LEFT},{3,4,5,RIGHT},{4,3,4,LEFT},
	    {4,5,6,RIGHT},{5,0,1,LEFT},{5,4,7,LEFT}}},
    {'0',12,{{0,1,6,RIGHT},{1,0,1,RIGHT},{1,2,5,LEFT},{1,6,7,RIGHT},
	    {2,1,2,LEFT},{2,5,6,LEFT},{3,1,2,RIGHT},{3,5,6,RIGHT},
	    {4,0,1,LEFT},{4,2,5,RIGHT},{4,6,7,LEFT},{5,1,6,LEFT}}},
    {'1',6,{{0,0,1,RIGHT},{0,5,6,RIGHT},{1,6,7,RIGHT},
	    {2,1,5,RIGHT},{3,1,7,LEFT},{5,0,1,LEFT}}},
    {'2',13,{{0,0,2,RIGHT},{0,5,6,RIGHT},{1,2,3,RIGHT},{1,6,7,RIGHT},
	    {2,1,2,LEFT},{2,3,4,RIGHT},{2,5,6,LEFT},{3,2,3,LEFT},
	    {3,4,6,RIGHT},{4,3,4,LEFT},{4,6,7,LEFT},{5,0,1,LEFT},
	    {5,4,6,LEFT}}},
    {'3',6,{{0,0,1,RIGHT},{0,6,7,RIGHT},{2,3,4,RIGHT},
	    {4,1,3,RIGHT},{4,4,6,RIGHT},{5,0,7,LEFT}}},
    {'4',7,{{0,1,7,RIGHT},{1,2,7,LEFT},{3,0,1,RIGHT},{3,2,3,RIGHT},
	    {4,0,1,LEFT},{4,2,3,LEFT},{5,1,2,LEFT}}},
    {'5',12,{{0,1,2,RIGHT},{0,4,7,RIGHT},{1,0,1,RIGHT},{1,5,6,LEFT},
	    {2,1,2,LEFT},{3,1,2,RIGHT},{3,3,4,RIGHT},{4,0,1,LEFT},
	    {4,2,3,RIGHT},{4,4,5,LEFT},{5,1,4,LEFT},{5,6,7,LEFT}}},
    {'6',6,{{0,0,7,RIGHT},{1,1,3,LEFT},{1,4,6,LEFT},
	    {4,1,3,RIGHT},{5,0,4,LEFT},{5,6,7,LEFT}}},
    {'7',8,{{0,6,7,RIGHT},{1,0,2,RIGHT},{2,0,1,LEFT},{2,2,4,RIGHT},
	    {3,1,3,LEFT},{3,4,6,RIGHT},{4,3,5,LEFT},{5,5,7,LEFT}}},
    {'8',6,{{0,0,7,RIGHT},{1,1,3,LEFT},{1,4,6,LEFT},
	    {4,1,3,RIGHT},{4,4,6,RIGHT},{5,0,7,LEFT}}},
    {'9',6,{{0,0,1,RIGHT},{0,3,7,RIGHT},{1,4,6,LEFT},
	    {4,1,3,RIGHT},{4,4,6,RIGHT},{5,0,7,LEFT}}},
    {'+',6,{{0,3,4,RIGHT},{2,1,3,RIGHT},{2,4,6,RIGHT},
	    {3,1,3,LEFT},{3,4,6,LEFT},{5,3,4,LEFT}}},
    {'-',2,{{0,3,4,RIGHT},{5,3,4,LEFT}}},
    {' ',0}
};

void proc_text (int height, char *string)
{
    struct charedge   *c_edgeptr;
    struct charstruct *c_structptr;
    int    textoffset, textmul;
    register int    i;
    register char  *cp;

    textoffset = 0;
    textmul = (height + 69) / 70;
    if (textmul <= 0) textmul = 1;
    textmul *= 10;
    for (cp = string; *cp != '\0'; ++cp) {
	for (i = 0; *cp != text[i].character; ++i);
	c_structptr = &text[i];
	for (i = c_structptr -> nrofedges - 1; i >= 0; --i) {
	    c_edgeptr = &c_structptr -> edge[i];
	    sort_ins_edge (
		c_edgeptr -> x * textmul + textoffset,
		c_edgeptr -> ybottom * textmul,
		c_edgeptr -> ytop * textmul,
		c_edgeptr -> bodybit);
	}
	textoffset += textmul * 7;
    }
}
