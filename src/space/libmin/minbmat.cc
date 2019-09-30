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

#include <stdlib.h>
#include <string.h>

#include "src/space/libmin/minbmat.h"

//------------------------------------------------------------------------------

MINBitMatrix::MINBitMatrix(STDDWord num_cols, STDDWord rows_allocated)
	: _num_rows(0),
	  _num_cols(num_cols),
	  _rows_allocated(rows_allocated)
{
	// calculate number of dwords per row
	//
	_dwords_per_row = (_num_cols+31)/32;

	STDDWord size = _rows_allocated * _dwords_per_row * sizeof(STDDWord);
	_data = (STDDWord*) malloc(size?size:1);
	STD_MEMORY(_data);
}

MINBitMatrix::~MINBitMatrix(void)
{
	if(_data) free(_data);
}

//------------------------------------------------------------------------------

// task: resize allocated data; the specified number of allocated rows may not
//       be less than the number of currently stored rows;
//
void
MINBitMatrix::resize(STDDWord rows_allocated)
{
	STD_ASSERT(rows_allocated >= _num_rows);

	_rows_allocated = rows_allocated;

	STDDWord size = _rows_allocated * _dwords_per_row * sizeof(STDDWord);
	_data = (STDDWord*) realloc(_data, size?size:1);
	STD_MEMORY(_data);
}

// task: perform a resize, but only if necessary;
//
void
MINBitMatrix::reshape(STDDWord rows_allocated)
{
	// note: reshape only resizes when necessary
	//
	if(rows_allocated > _rows_allocated) resize(rows_allocated*2);
}

//------------------------------------------------------------------------------

// task: return the value of a single bit from within the matrix
//
STDDWord
MINBitMatrix::getBit(STDDWord row, STDDWord col) const
{
	STD_ASSERT(row < _num_rows);
	STD_ASSERT(col < _num_cols);

	STDDWord* w = _data + (row * _dwords_per_row);
	w += col/32;

	int shift = col%32;

	return (*w & (STDDWord(1)<<shift))?1:0;
}

// task: set the value of a single bit
//
void
MINBitMatrix::setBit(STDDWord row, STDDWord col, STDDWord bit)
{
	STD_ASSERT(row >= 0 && row < _num_rows);
	STD_ASSERT(col >= 0 && col < _num_cols);

	STDDWord* w = _data + (row * _dwords_per_row);
	w += col/32;

	int shift = col%32;

	*w = (*w&~(STDDWord(1)<<shift)) | (bit<<shift);
}

//------------------------------------------------------------------------------

// task: read a single row from a character string
//
void
MINBitMatrix::setRow(STDDWord row, const char* string)
{
	const char* s = string;

	for(STDDWord col = 0; col < _num_cols; col++)
	{
		STD_ASSERT(*s == '0' || *s == '1');

		setBit(row, col, *s - '0'); ++s;
	}
}

// task: add a row, and read it's contents from a character string
//
void
MINBitMatrix::addRow(const char* string)
{
	// reshape data
	//
	reshape(_num_rows+1);
	_num_rows++;

	setRow(_num_rows-1, string);
}

// task: add a specified number of rows; all bits are set to 0
//
void
MINBitMatrix::addRows(STDDWord n)
{
	reshape(_num_rows+n);

	STDDWord* w = _data + _num_rows * _dwords_per_row;
	memset(w, 0, n * _dwords_per_row * sizeof(STDDWord));

	_num_rows += n;
}

// task: add a single row; all bits are set to 0;
//
void
MINBitMatrix::addRow(void)
{
	addRows(1);
}

//------------------------------------------------------------------------------

// task: output the matrix in human-readable format
//
ostream& operator<<(ostream& ostr, const MINBitMatrix& bm)
{
	for(STDDWord row = 0; row < bm._num_rows; row++)
	{
		for(STDDWord col = 0; col < bm._num_cols; col++)
		{
			ostr << char('0'+bm.getBit(row, col));
		}
		ostr << endl;
	}
	return ostr;
}

//------------------------------------------------------------------------------
