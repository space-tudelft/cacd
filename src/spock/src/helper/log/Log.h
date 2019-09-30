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
#ifndef _LOG_H_
#define _LOG_H_

//============================================================================
//! Provides logging to screen and file. Supports sectioned logging.
/*! \file

    This file contains the Log class. Log provides a logging mechanism.
    Logging is done to file and optionally to screen.

    \author Arjen van Rhijn
*/
//============================================================================

#include <qmultilinedit.h>

// include files
#include <fstream>
#include <string>
#include <map>

using namespace std;

#define __LOGGING__

#ifdef __LOGGING__

//! A class that implements sectioned logging to file and screen.
/*!
    The Log class provides a logging mechanism. Logging is done to file and
    optionally to screen.

    \author Arjen van Rhijn
*/
class Log
{
    public:
	static void     Write(const char message[], ...);

	static void     EnableScreen();
	static void     DisableScreen();

	static void     ReadLoglist(const std::string& fname);
	static void     SetFile(const std::string& fname);
	static void     SetWindow(QMultiLineEdit* edit);

    private:
	// stream management
	static void     Open();
	static void     Close();

	// static log data
	static bool     m_create;       //!< If true, a new file is started, false appends.
	static bool     m_logging;      //!< If true, logging is enabled.
	static bool     m_logscr;       //!< Log to screen if true.
	static std::string   m_fname;

	static ofstream         m_output;  //!< The output stream.
	static std::map<std::string, int> m_filetab;
	static QMultiLineEdit*  m_edit;
};

// this works with gcc only...
// note the open else statement, this is to make statements as
// if (x) LOG(a) else if (y) LOG(b); possible
#define LOG(a...) if (true) { Log::SetFile(__FILE__); Log::Write(## a); } else

// you can use this macro if you're using another compiler, make sure you use
// double ()'s when using the macro, like this:
// LOG(("Variable p = %i\n", p));
//#define LOG(a) if (true) { Log::SetFile(__FILE__); Log::Write a; } else

#else

// disabled
#define LOG //

#endif

#endif
