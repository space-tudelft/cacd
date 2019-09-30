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

#include <algorithm>
#include <new>

#include <stdlib.h>
#include <string.h>

#include "src/space/libmin/mintable.h"
#include "src/space/libmin/mincover.h"
#include "src/space/libmin/mincompl.h"
#include "src/space/libmin/mintaut.h"
#include "src/space/libmin/minsym.h"

//------------------------------------------------------------------------------
// References
//
// [1] De Micheli, G., "Synthesis and optimization of digital circuits,"
//     McGraw-Hill, 1994.
//
//------------------------------------------------------------------------------

MINTable::MINTable(STDDWord num_vars, STDDWord rows_allocated)
	: _num_vars(num_vars),
	  _num_rows(0),
	  _rows_allocated(rows_allocated)
{
	_dwords_per_row = (_num_vars+15)/16;

	STDDWord size = _rows_allocated * _dwords_per_row * sizeof(STDDWord);
	_data = (STDDWord*) malloc(size?size:1);
	STD_MEMORY(_data);
}

MINTable::MINTable(const MINTable& table)
	: _num_vars(table._num_vars),
	  _num_rows(table._num_rows),
	  _dwords_per_row(table._dwords_per_row),
	  _rows_allocated(table._rows_allocated)
{
	STDDWord size = _rows_allocated * _dwords_per_row * sizeof(STDDWord);
	_data = (STDDWord*) malloc(size?size:1);
	STD_MEMORY(_data);

	memcpy(_data, table._data, _num_rows * _dwords_per_row * sizeof(STDDWord));
}

MINTable::~MINTable(void)
{
	if(_data) free(_data);
}

//------------------------------------------------------------------------------

// task: resize allocated data; the specified number of allocated rows may not
//       be less than the number of currently stored rows;
//
void
MINTable::resize(STDDWord rows_allocated)
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
MINTable::reshape(STDDWord rows_allocated)
{
	// note: reshape only resizes when necessary
	//
	if(rows_allocated > _rows_allocated)
		resize(rows_allocated*2);
}

//------------------------------------------------------------------------------

// task: assign the contents of another table to this table; the number of
//       variables of both tables should be equal;
//
MINTable&
MINTable::operator=(const MINTable& table)
{
	// note: number of variables must be the same
	//
	STD_ASSERT(_num_vars == table._num_vars);

	reshape(table._num_rows);
	_num_rows = table._num_rows;

	// note: use memmove(), because tables might be one and the same object
	//
	memmove(_data, table._data, _num_rows * _dwords_per_row * sizeof(STDDWord));

	return *this;
}

//------------------------------------------------------------------------------

// task: return a two-bit field from within the table
//
STDDWord
MINTable::getField(STDDWord row, STDDWord var) const
{
	STD_ASSERT(var < _num_vars);
	STD_ASSERT(row < _num_rows);

	STDDWord* w = _data + _dwords_per_row*row;
	w += var/16;

	int shift = (var%16)*2;

	return STDDWord((*w >> shift) & 0x3);
}

// task: set a two-bit field
//
void
MINTable::setField(STDDWord row, STDDWord var, STDDWord field)
{
	STD_ASSERT(var < _num_vars);
	STD_ASSERT(row < _num_rows);
	STD_ASSERT((field & 0x3) == field);

	STDDWord* w = _data + _dwords_per_row*row;
	w += var/16;

	int shift = (var%16)*2;

	*w = (*w & ~STDDWord(0x3L << shift)) | (field << shift);
}

// task: return a single bit from within the table
//
STDDWord
MINTable::getBit(STDDWord row, STDDWord var, STDDWord p) const
{
	STD_ASSERT(p <= 1);
	return STDDWord((getField(row, var) >> p)&1);
}

// task: set a single bit
//
void
MINTable::setBit(STDDWord row, STDDWord var, STDDWord p, STDDWord bit)
{
	STD_ASSERT(p <= 1);
	STD_ASSERT(var < _num_vars);
	STD_ASSERT(row < _num_rows);
	STD_ASSERT((bit & 0x1) == bit);

	STDDWord* w = _data + _dwords_per_row*row;
	w += var/16;

	int shift = ((var % 16)*2)+p;

	*w = (*w & ~STDDWord(0x1L << shift)) | (bit << shift);
}

//------------------------------------------------------------------------------

// task: add an empty row to the table; note that this leaves the table in an
//       improper state (because no field may be empty);
//
void
MINTable::addRow(void)
{
	reshape(_num_rows + 1);

	STDDWord* w = _data + _num_rows * _dwords_per_row;
	memset(w, 0, _dwords_per_row * sizeof(STDDWord));

	_num_rows++;
}

// task: append a row from another table to this table
//
void
MINTable::addRow(const MINTable& table, STDDWord row)
{
	STD_ASSERT(row < table._num_rows);
	STD_ASSERT(_num_vars == table._num_vars);

	reshape(_num_rows + 1);

	STDDWord* w = _data + _num_rows * _dwords_per_row;
	STDDWord amount = _dwords_per_row * sizeof(STDDWord);
	memcpy(w, table._data+row*table._dwords_per_row, amount);

	_num_rows++;
}

// task: append all rows from another table to this table
//
void
MINTable::addRows(const MINTable& table)
{
	reshape(_num_rows + table._num_rows);

	STDDWord* w = _data + _num_rows * _dwords_per_row;
	STDDWord amount = _dwords_per_row * table._num_rows * sizeof(STDDWord);
	memcpy(w, table._data, amount);

	_num_rows += table._num_rows;
}

// task: add a row from a character string
//
void
MINTable::addRow(const char* string)
{
	reshape(_num_rows + 1);

	STDDWord mask = 0x1;
	STDDWord* w = _data + _num_rows*_dwords_per_row;
	STDDWord store = 0;
	STDDWord num_bits = _num_vars * 2;

	while(num_bits > 0)
	{
		if(*string == '0')
		{
			num_bits--;
			mask <<= 1;
		}
		else if(*string == '1')
		{
			num_bits--;
			store |= mask;
			mask <<= 1;
		}
		else STD_ASSERT(*string); // not enough bits

		if(mask == 0)
		{
			*(w++) = store;
			store = 0;
			mask = 0x1;
		}

		string++;
	}

	if(mask != 0x1)
	{
		*(w++) = store;
	}

	_num_rows++;
}

// task: add the universal cube, i.e. the cube with all 1s, to this table
//
void
MINTable::addUniversalCube(void)
{
	STDDWord row = _num_rows;

	addRow();

	for(STDDWord var = 0; var < _num_vars; var++)
		setField(row, var, 0x3);
}

// task: remove a row from this table
//
void
MINTable::removeRow(STDDWord row)
{
	STD_ASSERT(row < _num_rows);

	STDDWord* w = _data + row * _dwords_per_row;
	STDDWord amount = (_num_rows-row-1) * _dwords_per_row * sizeof(STDDWord);
	memmove(w, w+_dwords_per_row, amount);

	_num_rows--;
}

//------------------------------------------------------------------------------

// task: return a single row from this table
//
MINTable
MINTable::getRow(STDDWord row) const
{
	STD_ASSERT(row < _num_rows);

	MINTable table(_num_vars, 1);
	table.addRow(*this, row);

	return table;
}

//------------------------------------------------------------------------------

// task: remove all rows from this table
//
void
MINTable::clear(void)
{
	_num_rows = 0;
}

//------------------------------------------------------------------------------

// task: verify the integrity of this table
//
bool
MINTable::integrity(void) const
{
	for(STDDWord row = 0; row < _num_rows; row++)
	{
		for(STDDWord var = 0; var < _num_vars; var++)
		{
			STDDWord field = getField(row, var);
			if(field == 0) return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------------

// task: calculate a complementary cover
//
MINTable
MINTable::complement(void)
{
	MINTable result(_num_vars, _num_rows);

	MINComplementSolver solver(*this);
	solver.solve(result);

	return result;
}

//------------------------------------------------------------------------------

// task: determine whether the current cover is a tautology;
//
bool
MINTable::tautology(void) const
{
	MINTautologySolver solver(*this);
	return solver.solve();
}

//------------------------------------------------------------------------------

// task: determine the cofactor of this table with respect to a row of a given
//       table;
//
MINTable
MINTable::cofactor(const MINTable& table, STDDWord row) const
{
	MINTable result(_num_vars, _num_rows);

	for(STDDWord r = 0; r < _num_rows; r++)
	{
		if(intersects(*this, r, table, row))
		{
			result.reshape(result._num_rows+1);
			result._num_rows++;

			for(STDDWord var = 0; var < _num_vars; var++)
			{
				STDDWord field1 = getField(r, var);
				STDDWord field2 = table.getField(row, var);

				result.setField(result._num_rows-1, var,
						field1 | ((~field2)&0x3));
			}
		}
	}

	return result;
}

//------------------------------------------------------------------------------

// task: determine whether two rows intersect;
//
bool
MINTable::intersects(const MINTable& table1, STDDWord row1,
		     const MINTable& table2, STDDWord row2)
{
	STD_ASSERT(table1._num_vars == table2._num_vars);

	for(STDDWord var = 0; var < table1._num_vars; var++)
	{
		STDDWord field1 = table1.getField(row1, var);
		STDDWord field2 = table2.getField(row2, var);

		if(!(field1 & field2))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------------

// task: determine whether the current cover (this table) covers a single
//       row;
//
bool
MINTable::covers(const MINTable& table, STDDWord row) const
{
	// note: create cofactor with respect to the implicant
	//
	MINTable cofactor = MINTable::cofactor(table, row);

	// note: determine whether the resulting function is a tautology
	//
	return cofactor.tautology() ? true : false;
}

// task: determine whether the current cover (this table) covers another table;
//       there may be far more efficient ways to obtain the same result;
//
bool
MINTable::covers(const MINTable& table) const
{
	for(STDDWord row = 0; row < table._num_rows; row++)
	{
		if(!covers(table, row))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------------

// task: test whether two tables are equivalent; note that there may be far more
//       efficient ways to obtain the same result;
//
bool
MINTable::isEquivalent(const MINTable& table) const
{
	return (covers(table) && table.covers(*this)) ? true : false;
}

//------------------------------------------------------------------------------

// note: this structure is used to sort rows according to some metric;
//
struct MINSortStruct
{
	STDDWord value;
	STDDWord data;

	bool operator>(const MINSortStruct& ss) const
	{ return value > ss.value; }
	bool operator<(const MINSortStruct& ss) const
	{ return value < ss.value; }

        static bool less_than(const MINSortStruct& ss1, const MINSortStruct& ss2)
        { return ss1 < ss2; }
        static bool greater_than(const MINSortStruct& ss1, const MINSortStruct& ss2)
        { return ss1 > ss2; }
};

// task: sort rows according to the row weights; see getRowWeight();
//
void
MINTable::sortRows(bool ascending, std::vector<STDDWord>& result) const
{
	// get sums of columns
	//
	std::vector<STDDWord> column_sums;
        column_sums.reserve(_num_vars*2);
	getColumnSums(column_sums);

	// sort rows
	//
	std::vector<MINSortStruct> sorted_rows;
        sorted_rows.reserve(_num_rows);
	for(STDDWord row = 0; row < _num_rows; row++)
	{
		MINSortStruct ss;
		ss.value = getRowWeight(row, column_sums);
		ss.data = row;

		sorted_rows.push_back(ss);
	}

        if(ascending)
            std::sort(sorted_rows.begin(), sorted_rows.end(), MINSortStruct::less_than);
        else
            std::sort(sorted_rows.begin(), sorted_rows.end(), MINSortStruct::greater_than);

	for(STDDWord index = 0; index < _num_rows; index++)
	{
		result.push_back(sorted_rows[index].data);
	}
}

// task: calculate the sums of entries in each column;
//
void
MINTable::getColumnSums(std::vector<STDDWord>& result) const
{
        result.clear();
        result.resize(_num_vars*2, 0);

	for(STDDWord col = 0; col < _num_vars*2; col++)
	{
		STDDWord sum = 0;

		for(STDDWord row = 0; row < _num_rows; row++)
		{
			STDDWord bit = getBit(row, col/2, col%2);
			sum += bit;
		}
		result[col] = sum;
	}
}

// task: calculate the weight of a single row, which is defined as the
//       dot-product of the column_sums vector and that row; see also
//       getColumnSums();
//
STDDWord
MINTable::getRowWeight(STDDWord row, std::vector<STDDWord>& column_sums) const
{
	STDDWord result = 0;

	for(STDDWord col = 0; col < _num_vars*2; col++)
	{
		STDDWord bit = getBit(row, col/2, col%2);

		if(bit)
			result += column_sums[col];
	}

	return result;
}

//------------------------------------------------------------------------------

// task: expand this table by raising as much 0s to 1s as possible;
//       see section 7.4.1. of [1] for further details;
//
void
MINTable::expand(const MINTable& off_table)
{
#ifdef __VERBOSE__
	cerr << "=== EXPAND STAGE ===\n";
#endif

	// sort rows (ascending order)
	//
	std::vector<STDDWord> sorted_rows;
        sorted_rows.reserve(_num_rows);
	sortRows(true, sorted_rows);

	// find zero columns in the OFF-set
	//
	std::vector<STDDWord> zero_columns;
        zero_columns.reserve(_num_vars*2);
	off_table.findZeroColumns(zero_columns);

	// expand rows in the calculated order
	//
	for(STDDWord index = 0; index < _num_rows; index++)
	{
		STDDWord row = sorted_rows[index];
		expand(row, zero_columns, off_table);
	}
}

// task: expand a single row;
//       see section 7.4.1. of [1] for further details;
//
void
MINTable::expand(STDDWord row, std::vector<STDDWord>& zero_columns,
		 const MINTable& off_table)
{
#ifdef __VERBOSE__
	cerr << "expanding row " << row << " ...\n";
#endif

	for(STDDWord i = 0; i < zero_columns.size(); i++)
	{
		STDDWord col = zero_columns[i];
		setBit(row, col/2, col%2, 1);
	}

	// determine the free-set; these lines have been outcommented
	// because the free-set is implicitly handled by the
	// minimum cover problem solver; hence getFreeSet() may
	// eventually be removed from the source code;
	//
	// STDIntSet free_set(_num_vars*2);
	// getFreeSet(row, off_table, free_set);

	// create cover matrix
	//
	MINCover* cover = createExpandCover(row, off_table);

#ifdef __VERBOSE__
	cerr << "COVER\n" << *cover;
#endif

	// solve minimum cover problem
	//
	STDIntSet distance = cover->minimize();

#ifdef __VERBOSE__
	cerr << "minimized cover: " << distance << endl;
#endif

	// fields not in the cover may now be complemented
	//
	for(STDDWord var = 0; var < _num_vars; var++)
	{
		if(!distance.contains(var))
			setField(row, var, 0x3);
	}

	delete cover;
}

// task: find columns containing all 0s within this table;
//
void
MINTable::findZeroColumns(std::vector<STDDWord>& result) const
{
	for(STDDWord col = 0; col < _num_vars*2; col++)
	{
		STDDWord row;

		for(row = 0; row < _num_rows; row++)
		{
			STDDWord bit = getBit(row, col/2, col%2);
			if(bit) break;
		}

		if(row == _num_rows)
			result.push_back(col);
	}
}

// task: calculate the free-set, i.e. the set of columns in which a zero may
//       be raised; see section 7.4.1. of [1] for further details;
//
// note: this method is obsolete (see the expand() method)
//
void
MINTable::getFreeSet(STDDWord row, const MINTable& off_table,
		     STDIntSet& result) const
{
	// fill the free-set
	//
	for(STDDWord col = 0; col < _num_vars*2; col++)
	{
		STDDWord bit = getBit(row, col/2, col%2);

		if(bit == 0)
			result.remove(col);
	}

	// remove from the free-set all columns which can never be raised
	//
	for(STDDWord r = 0; r < off_table._num_rows; r++)
	{
		STDSDWord loc = -1;

		for(STDDWord var = 0; var < _num_vars; var++)
		{
			STDDWord field1 = getField(row, var);
			STDDWord field2 = off_table.getField(r, var);

			if((field1 & field2) == 0)
			{
				if(loc < 0)
				{
					loc = var;
				}
				else
				{
					loc = -2;
					break;
				}
			}
		}

		// note: row may not intersect the OFF-set
		//
		STD_ASSERT(loc != -1);

		if(loc >= 0)
		{
			STDDWord field = getField(row, loc);
			STDDWord col;

			// note: field must have exactly one bit set
			//
			if(field == 0x1)
				col = loc*2+1;
			else {
				STD_ASSERT(field == 0x2);
				col = loc*2;
			}

#ifdef __VERBOSE__
			cerr << "column " << col << " may not be set\n";
#endif

			result.remove(col);
		}
	}
}

// task: create a cover-matrix corresponding to the expansion of a single row;
//       see section 7.4.1. of [1] for further details;
//
MINCover*
MINTable::createExpandCover(STDDWord row, const MINTable& off_table) const
{
	// note: create a new cover matrix
	//
	MINCover* cover = new MINCover(_num_vars, off_table._num_rows);
	STD_MEMORY(cover);
	cover->addRows(off_table._num_rows);

	for(STDDWord r = 0; r < off_table._num_rows; r++)
	{
		for(STDDWord var = 0; var < _num_vars; var++)
		{
			STDDWord field1 = getField(row, var);
			STDDWord field2 = off_table.getField(r, var);

			// note: if fields do not intersect, a bit is entered
			//       into the cover matrix
			//
			if((field1 & field2) == 0)
			{
				cover->setBit(r, var, 1);
			}
		}
	}

	return cover;
}

//------------------------------------------------------------------------------

// task: create a new table with as little rows from this table as possible,
//       under the restriction that the new table should still cover the
//       function corresponding to this table;
//       see section 7.4.3. of [1] for further details;
//
MINTable
MINTable::irredundant(void) const
{
        STDDWord row;
        STDDWord alpha;

#ifdef __VERBOSE__
	cerr << "=== IRREDUNDANT STAGE ===\n";
#endif

	MINTable result(_num_vars, _num_rows);

	// note: calculate the relatively essential set, and the totally
	//       redundant set;
	//
	STDIntSet er_set = relativelyEssentialSet();
	STDIntSet tr_set = totallyRedundantSet(er_set);

	// get the cardinality of er_set
	//
	STDDWord er_size = er_set.get_cardinality();

	// create a sorted table containing implicants not in tr_set; the first
	// implicants of the table will belong to er_set
	//
	MINTable table(_num_vars, _num_rows - tr_set.get_cardinality());

	for(row = 0; row < _num_rows; row++)
	{
		if(er_set.contains(row))
			table.addRow(*this, row);
	}
	for(row = 0; row < _num_rows; row++)
	{
		if(!(tr_set.contains(row) || er_set.contains(row)))
			table.addRow(*this, row);
	}

#ifdef __VERBOSE__
	cerr << "TABLE (without totally redundant implicants)\n" << table;
	cerr << "(first " << er_size << " are relatively essential)\n\n";
#endif

	// now consider the removal of each implicant `alpha' not in er_set; keep
	// a cover matrix to register the implications of this removal
	//
	MINCover cover(table._num_rows - er_size, 2*table._num_rows);

	for(alpha = er_size; alpha < table._num_rows; alpha++)
	{
		STDIntSet removed;
                removed.reserve(table._num_rows);
		removed.insert(alpha);

		// note: implicitly, `h' is the table without implicant alpha;
		//       then `h_alpha' is the cover-table of the cofactor of
		//       `h' with respect to alpha;
		//       the following loop calculates h_alpha;
		//
		MINTable h_alpha(table);

		for(row = 0; row < table._num_rows; row++)
		{
			if(row == alpha) continue;

			if(!intersects(table, alpha, table, row))
			{
				removed.insert(row);
				continue;
			}

			for(STDDWord var = 0; var < _num_vars; var++)
			{
				STDDWord field1 = table.getField(row, var);
				STDDWord field2 = table.getField(alpha, var);

				h_alpha.setField(row, var, field1 | ((~field2)&0x3));
			}
		}

#ifdef __VERBOSE__
		cerr << "H_ALPHA (alpha = " << alpha << ")\n" << h_alpha;
		cerr << "removed: " << removed << endl << endl;
#endif

		MINTautologyCoverSolver solver(h_alpha, cover, alpha, er_size);
		solver.removeRows(removed);

		bool tautology = solver.solve();
		STD_ASSERT(tautology);

#ifdef __VERBOSE__
		cerr << "cover (so far)\n" << cover << endl;
#endif
	}

	// minimize the cover
	//
	STDIntSet min_cover = cover.minimize();

	// build the result-matrix
	//
	for(row = 0; row < er_size; row++)
	{
		result.addRow(table, row);
	}
	for(row = er_size; row < table._num_rows; row++)
	{
		if(min_cover.contains(row-er_size))
			result.addRow(table, row);
	}

	return result;
}

// task: return the set of relatively essential implicants;
//       see section 7.4.3. of [1] for further details;
//
STDIntSet
MINTable::relativelyEssentialSet(void) const
{
	STDIntSet result;
        result.reserve(_num_rows);

	for(STDDWord row = 0; row < _num_rows; row++)
	{
		// copy this table
		//
		MINTable table(*this);

		// remove current row from table
		//
		table.removeRow(row);

		// check if rest of the table covers the row
		//
		if(!table.covers(*this, row))
			result.insert(row);
	}

	return result;
}

// task: return the set of totally redundant implicants;
//       see section 7.4.3. of [1] for further details;
//
STDIntSet
MINTable::totallyRedundantSet(const STDIntSet& er_set) const
{
        STDDWord row;

	STDIntSet result;
        result.reserve(_num_rows);

	// setup table of relatively essential implicants
	//
	MINTable er_table(_num_vars, er_set.get_cardinality());

	for(row = 0; row < _num_rows; row++)
	{
		if(er_set.contains(row))
			er_table.addRow(*this, row);
	}

	for(row = 0; row < _num_rows; row++)
	{
		if(er_set.contains(row)) continue;

		if(er_table.covers(*this, row))
			result.insert(row);
	}

	return result;
}

//------------------------------------------------------------------------------

// task: drop as much 1s from the table as possible;
//       see section 7.4.2. of [1] for further details;
//
void
MINTable::reduce(void)
{
#ifdef __VERBOSE__
	cerr << "=== REDUCE STAGE ===\n";
#endif

	// sort rows (descending order)
	//
	std::vector<STDDWord> sorted_rows;
        sorted_rows.reserve(_num_rows);
	sortRows(false, sorted_rows);

	for(STDDWord index = 0; index < _num_rows; index++)
	{
		STDDWord row = sorted_rows[index];
		reduce(row);
	}
}

// task: reduce a single row
//       see section 7.4.2. of [1] for further details;
//
void
MINTable::reduce(STDDWord row)
{
#ifdef __VERBOSE__
	cerr << "reducing row " << row << " ...\n";
#endif

	// note: create a new cover without the current row
	//
	MINTable table(*this);
	table.removeRow(row);

	// note: take the complement of the cofactor with respect to the
	//       current row
	//
	MINTable cofactor = table.cofactor(*this, row);
	MINTable complem = cofactor.complement();

	// note: reduce the current row using the complement
	//
	for(STDDWord col = 0; col < 2*_num_vars; col++)
	{
		if(!getBit(row, col/2, col%2))
			continue;

		STDDWord bit = 0;

		for(STDDWord r = 0; r < complem._num_rows; r++)
		{
			if(complem.getBit(r, col/2, col%2))
			{
				bit = 1;
				break;
			}
		}
		setBit(row, col/2, col%2, bit);
	}
	STD_ASSERT(integrity());
}

//------------------------------------------------------------------------------

// task: calculate the cost of this cover;
//
STDDWord
MINTable::cost(void) const
{
	STDDWord cost = 0;

	for(STDDWord row = 0; row < _num_rows; row++)
	{
		for(STDDWord var = 0; var < _num_vars; var++)
		{
			if(getField(row, var) != 0x3) cost++;
		}
	}
	return cost;
}

//------------------------------------------------------------------------------

// task: minimize the cost of this table; the method resembles the structure of
//       the routine shown on page 316 of [1]; note, however, that the
//       conditions in the do..while loops have been reversed;
//
void
MINTable::minimize(void)
{
	MINTable complem = complement();
	expand(complem);
	*this = irredundant();

	STDDWord phi1;
	STDDWord phi2;

	do
	{
		phi2 = cost();

		do
		{
			phi1 = _num_rows;
			reduce();
			expand(complem);
			*this = irredundant();
		}
		while(_num_rows < phi1);
	}
	while(cost() < phi2);
}

//------------------------------------------------------------------------------

// task: output the table in human-readable format;
//
ostream& operator<<(ostream& ostr, const MINTable& table)
{
	for(STDDWord row = 0; row < table._num_rows; row++)
	{
		for(STDDWord var = 0; var < table._num_vars; var++)
		{
			if(var != 0) ostr << " ";

			for(STDDWord p = 0; p < 2; p++)
			{
				ostr << char('0' + table.getBit(row, var, p));
			}
		}
		ostr << endl;
	}
	return ostr;
}

//------------------------------------------------------------------------------

// task: output the table as a (human-readable) expression

void
MINTable::outputAsExpression(ostream& ostr, const std::vector<MINSymbol*>& variables)
{
    STD_ASSERT(variables.size() == _num_vars);

    if(_num_rows == 0) { ostr << "0"; return; }

    for(STDDWord row = 0; row < _num_rows; ++row)
    {
        if(row > 0) ostr << " | ";

        bool printed_primary = false;
        for(STDDWord var = 0; var < _num_vars; ++var)
        {
            STDDWord f = getField(row, var);

            if(f == 0x3) continue;

            if(printed_primary) ostr << " ";
	    else printed_primary = true;

            if(f == 0x1) ostr << "!";
	    else STD_ASSERT(f == 0x2);

            ostr << variables[var]->get_name();
        }

        if(!printed_primary) { ostr << "1"; return; }
    }
}

//------------------------------------------------------------------------------

