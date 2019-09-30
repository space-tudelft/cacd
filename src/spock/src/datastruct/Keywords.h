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
#ifndef __PARSER_KEYWORDS_H__
#define __PARSER_KEYWORDS_H__

// The keywords below are also specified in lex.l
// If you decide to make any changes to the keywords,
// make sure you change lex.l as well.

#define KEY_DEFINE            "define"
#define KEY_INTEGER           "integer"
#define KEY_REAL              "real"
#define KEY_IDENTIFIER        "identifier"
#define KEY_CONDITIONLIST     "conditionlist"
#define KEY_STRING            "string"
#define KEY_COLOR             "color"
#define KEY_DALISTYLE         "dalistyle"
#define KEY_ITEM              "item"
#define KEY_DROPDOWN          "dropdown"
#define KEY_COMBOBOX          "combobox"
#define KEY_SPREADSHEET       "spreadsheet"
#define KEY_SECTION           "section"
#define KEY_MULTISECTION      "scrollframe"
#define KEY_TABPAGE           "tabpage"
#define KEY_PARAMLIST         "paramlist"
#define KEY_GENERATOR         "generator"
#define KEY_GENERATE          "generate"
#define KEY_MAP               "map"
#define KEY_FOREACH           "foreach"
#define KEY_INF               "inf"
#define KEY_APPLICATION       "application"
#define KEY_IF                "if"
#define KEY_IFCONDITION       "ifcond"

#define PROP_TITLE            "title"
#define PROP_HINT             "hint"
#define PROP_DEFAULT          "default"
#define PROP_ALIGN            "align"
#define PROP_FILENAME         "filename"
#define PROP_DATASOURCE       "adddatafrom"
#define PROP_UNIT             "unit"
#define PROP_VALIDATOR        "validator"
#define PROP_PIXMAP           "pixmap"
#define PROP_IMPLEMENT        "option"
#define PROP_DEFINE           "define"        // application internal use only
#define PROP_ITERIDENT        "iterident"     // application internal use only

// These are all special generator component properties/types.
#define GEN_LITERAL          "literal"
#define GEN_GENERATOR        "generator"
#define GEN_CONTEXT          "context"
#define GEN_ITERATOR         "iterator"
#define GEN_APPLICATION      "application"
#define GEN_VALUE            "value"
#define GEN_FIELD            "field"
#define GEN_FOREACH          "foreach"
#define GEN_INTEGER          "integer"
#define GEN_REAL             "real"
#define GEN_IDENTIFIER       "identifier"
#define GEN_IF               "if"

#define FIELD_COLOR          "clr"    // color is already a keyword :(
#define FIELD_FILL           "fille"
#define FIELD_PROCESSNAME    "processname"
#define FIELD_PROCESSDESC    "processdesc"
#define FIELD_DATE           "date"
#define FIELD_TIME           "time"

#endif // __PARSER_KEYWORDS_H__
