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
#ifndef __GENERAL_LIBS_LIBSTD_TYPES_H__
#define __GENERAL_LIBS_LIBSTD_TYPES_H__

#ifdef __cplusplus
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace libmin {
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif

//==============================================================================
//
//  Basic types and constants.
//
//==============================================================================

typedef unsigned char       STDByte;
typedef unsigned int        STDWord;
typedef unsigned long       STDDWord;

typedef unsigned long       STDMWord; // Machine word.

typedef unsigned long       STDULong;
typedef unsigned long long  STDQWord;
typedef unsigned long       STDSize;

typedef char                STDSByte;
typedef int                 STDSWord;
typedef long                STDSDWord;
typedef long                STDLong;
typedef long long           STDSQWord;
typedef long                STDSSize;

typedef double              STDDouble;
typedef long double         STDLongDouble;

// Note: use `bool' for boolean types. In C, we have to be a bit
// careful, because we define a bool as an unsigned char.
//
#ifndef __cplusplus
#define bool unsigned char
#define true  1
#define false 0
#endif


/*---
    Note: These minimum and maximum values are also available as:

        #include <limits>
        ...
        std::numeric_limits<typename>::min()
        std::numeric_limits<typename>::max()
---*/

#ifdef __cplusplus

const STDByte  STD_MAX_BYTE  = (STDByte) ~(0U);
const STDWord  STD_MAX_WORD  = (STDWord) ~(0U);
const STDDWord STD_MAX_DWORD = ~(0UL);
const STDMWord STD_MAX_MWORD = ~(0UL);
const STDULong STD_MAX_ULONG = ~(0UL);
const STDQWord STD_MAX_QWORD = ~(0ULL);
const STDSize  STD_MAX_SIZE  = ~(0UL);

const STDByte  STD_MIN_BYTE  = 0;
const STDWord  STD_MIN_WORD  = 0;
const STDDWord STD_MIN_DWORD = 0;
const STDMWord STD_MIN_MWORD = 0;
const STDULong STD_MIN_ULONG = 0;
const STDQWord STD_MIN_QWORD = 0;
const STDSize  STD_MIN_SIZE  = 0;

const STDSByte  STD_MAX_SBYTE  = (STDSByte)(((STDByte)~(0U))>>1);
const STDSWord  STD_MAX_SWORD  = (STDSWord)(((STDWord)~(0U))>>1);
const STDSDWord STD_MAX_SDWORD = (STDSDWord)(((STDDWord)~(0UL))>>1);
const STDLong   STD_MAX_LONG   = (STDLong)(((STDLong)~(0UL))>>1);
const STDSQWord STD_MAX_SQWORD = (STDSQWord)(((STDQWord)~(0ULL))>>1);
const STDSSize  STD_MAX_SSIZE  = (STDSSize)(((STDSize)~(0UL))>>1);

const STDSByte  STD_MIN_SBYTE  = (STDSByte)(~(((STDByte)~(0U))>>1));
const STDSWord  STD_MIN_SWORD  = (STDSWord)(~(((STDWord)~(0U))>>1));
const STDSDWord STD_MIN_SDWORD = (STDSDWord)(~(((STDDWord)~(0UL))>>1));
const STDLong   STD_MIN_LONG   = (STDLong)(~(((STDLong)~(0UL))>>1));
const STDSQWord STD_MIN_SQWORD = (STDSQWord)(~(((STDQWord)~(0ULL))>>1));
const STDSSize  STD_MIN_SSIZE  = (STDSSize)(~(((STDSize)~(0UL))>>1));

#else   /* !defined(__cplusplus) */

#define STD_MAX_BYTE  ((STDByte) ~(0U))
#define STD_MAX_WORD  ((STDWord) ~(0U))
#define STD_MAX_DWORD (~(0UL))
#define STD_MAX_MWORD (~(0UL))
#define STD_MAX_ULONG (~(0UL))
#define STD_MAX_QWORD (~(0ULL))
#define STD_MAX_SIZE  (~(0UL))

#define STD_MIN_BYTE  (0)
#define STD_MIN_WORD  (0)
#define STD_MIN_DWORD (0)
#define STD_MIN_MWORD (0)
#define STD_MIN_ULONG (0)
#define STD_MIN_QWORD (0)
#define STD_MIN_SIZE  (0)

#define STD_MAX_SBYTE  ((STDSByte)(((STDByte)~(0U))>>1))
#define STD_MAX_SWORD  ((STDSWord)(((STDWord)~(0U))>>1))
#define STD_MAX_SDWORD ((STDSDWord)(((STDDWord)~(0UL))>>1))
#define STD_MAX_LONG   ((STDLong)(((STDLong)~(0UL))>>1))
#define STD_MAX_SQWORD ((STDSQWord)(((STDQWord)~(0ULL))>>1))
#define STD_MAX_SSIZE  ((STDSSize)(((STDSize)~(0UL))>>1))

#define STD_MIN_SBYTE  ((STDSByte)(~(((STDByte)~(0U))>>1)))
#define STD_MIN_SWORD  ((STDSWord)(~(((STDWord)~(0U))>>1)))
#define STD_MIN_SDWORD ((STDSDWord)(~(((STDDWord)~(0UL))>>1)))
#define STD_MIN_LONG   ((STDLong)(~(((STDLong)~(0UL))>>1)))
#define STD_MIN_SQWORD ((STDSQWord)(~(((STDQWord)~(0ULL))>>1)))
#define STD_MIN_SSIZE  ((STDSSize)(~(((STDSize)~(0UL))>>1)))

#endif  /* __cplusplus */


#ifdef __cplusplus
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* end of namespace */ };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif

#endif
