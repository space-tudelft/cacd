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

#include "src/space/libmin/STDIntSet.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace libmin {
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//==============================================================================
//
//  STDIntSet methods.
//
//==============================================================================

STDIntSet::STDIntSet(void)
{
}

STDIntSet::STDIntSet(const STDIntSet& other) : bits(other.bits)
{
}

STDIntSet::~STDIntSet(void)
{
}

STDIntSet&
STDIntSet::operator=(const STDIntSet& other)
{
    bits = other.bits;
    return *this;
}

STDIntSet&
STDIntSet::operator-=(const STDIntSet& other)
{
    STDSize size = bits.size();
    if(other.bits.size() < size)
        size = other.bits.size();

    for(STDSize w = 0; w < size; ++w)
        bits[w] &= ~other.bits[w];

    return *this;
}

bool
STDIntSet::contains(STDSize index) const
{
    STDSize w = STDSize(index/32);
    if(w >= bits.size()) return false;

    return bool((bits[w]>>(index%32))&1);
}

void
STDIntSet::insert(STDSize index)
{
    STDSize w = STDSize(index/32);
    if(w >= bits.size()) bits.resize(w+1, 0);

    bits[w] |= STDDWord(1) << (index%32);
}

void
STDIntSet::remove(STDSize index)
{
    STDSize w = index/32;
    if(w >= bits.size()) return;

    bits[w] &= ~(STDDWord(1)<<(index%32));
}

void
STDIntSet::clear(void)
{
    bits.clear();
}

STDSize
STDIntSet::pick(void) const
{
    STD_ASSERT(bits.size() > 0);

    STDDWord val = bits[0];
    STDDWord index = 0;
    STDDWord mask = 1;

    for(;;)
    {
        if(val & mask) return index;

        mask <<= 1;
        index++;

        if(mask == 0)
        {
            mask = 1;
            STD_ASSERT("Cannot pick() from an empty set." && STDSize(index/32) < bits.size());
            val = bits[index/32];
        }
    }
}

STDSize
STDIntSet::get_cardinality(void) const
{
    STDSize count = 0;
    for(STDSize i = 0; i < bits.size(); ++i)
    {
        STDDWord v = bits[i];
        for(STDDWord mask = 1; mask != 0; mask <<= 1)
        {
            if(v & mask)
                count++;
        }
    }
    return count;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* end of namespace */ };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//==============================================================================
//
//  Output.
//
//==============================================================================

std::ostream& operator<<(std::ostream& ostr, const libmin::STDIntSet& int_set)
{
    bool printed = false;

    ostr << '{';
    for(libmin::STDSize i = 0; i < int_set.bits.size()*32; ++i)
    {
        if(int_set.contains(i))
        {
            if(printed)
                ostr << ',';
            else
                printed = true;

            ostr << i;
        }
    }
    return ostr << '}';
}

