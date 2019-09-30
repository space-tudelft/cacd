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

#ifndef __MINDEFS_H__
#define __MINDEFS_H__

#ifdef __cplusplus
#define STD_EXTERN_C extern "C"
#else
#define STD_EXTERN_C extern
#endif

#include "src/space/libmin/STDtypes.h"

#define STD_ASSERT(c) if(!(c)) __std_assert_failed__(__FILE__, __LINE__, #c)
#define STD_MEMORY(p) if(!(p)) __std_out_of_memory__(__FILE__, __LINE__, #p)

// Note: use the gcc construct `...' for the following macro. We could also
// have used __VA_ARGS__. This is necessary, because a class name can be a
// template instantiation, containing commas.
//
#define STD_DECLARE_CLASS(class_name...) \
public: static const char* dynamic_class_name(void) { return #class_name; } \
private:

#define STD_NON_COPYABLE(class_name...)  \
private:                                 \
    class_name(const class_name& other); \
    class_name& operator=(const class_name& other);

// Declarations:
STD_EXTERN_C void __std_assert_failed__(const char *filename, int lineno, const char* cond);
STD_EXTERN_C void __std_out_of_memory__(const char *filename, int lineno, const char* expr);
STD_EXTERN_C void die(void);
STD_EXTERN_C void min_init_library(void);

// Initializations:
#ifdef __cplusplus
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace libmin {
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class STDIntSet;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* end of namespace */ };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class MINBitMatrix;

class MINCover;
class MINCoverState;

class MINTable;

class MINSolver;
class MINComplementSolver;
class MINTautologySolver;
class MINTautologyCoverSolver;

class MINSymbol;
class MINSymbolTable;

class MINRobdd;

#endif /* __cplusplus */

#endif
