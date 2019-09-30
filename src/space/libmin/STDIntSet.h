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
#ifndef __GENERAL_LIBS_LIBSTD_STDINTSET_H__
#define __GENERAL_LIBS_LIBSTD_STDINTSET_H__

#include <iostream>
#include <vector>

#include "src/space/libmin/mindefs.h"

//==============================================================================
//
//  Forward declarations.
//
//==============================================================================

std::ostream& operator<<(std::ostream& ostr, const libmin::STDIntSet& int_set);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace libmin {
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//==============================================================================
//
//  class STDIntSet.
//
//==============================================================================

class STDIntSet
{
    STD_DECLARE_CLASS(STDIntSet);

private:
    // Vector of bits.
    //
    std::vector<STDDWord> bits;

public:
    STDIntSet(void);
    STDIntSet(const STDIntSet& other);
   ~STDIntSet(void);

    STDIntSet& operator=(const STDIntSet& other);

    STDIntSet& operator-=(const STDIntSet& other);

    void reserve(STDSize r) { bits.reserve(STDSize((r+31)/32)); }

    bool contains(STDSize index) const;
    void insert(STDSize index);
    void remove(STDSize index);
    void clear(void);
    STDSize pick(void) const;
    STDSize get_cardinality(void) const;

    // output
    //
    friend std::ostream& ::operator<<(std::ostream& ostr, const STDIntSet& int_set);
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* end of namespace */ };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif
