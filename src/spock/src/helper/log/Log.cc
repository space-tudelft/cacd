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

// include files
#include <stdio.h>
#include "src/spock/src/helper/log/Log.h"
#include <string>
#include <fstream>
#include <iostream>
#include <stdarg.h>
#include "src/spock/src/helper/log/Helper.h"

using namespace std;

#ifdef __LOGGING__

// static log data
ofstream Log::m_output;
string   Log::m_fname;

bool     Log::m_create = true;
bool     Log::m_logscr = false;

map<string, int> Log::m_filetab;
QMultiLineEdit*  Log::m_edit = NULL;

void Log::SetWindow(QMultiLineEdit* edit)
{
    m_edit = edit;
}

//============================================================================
/*! Set the current file name for which the Log function is called
 *
 *  \param fname Name of source file
 */
void Log::SetFile(const string& fname)
{
    m_fname = fname;
}

//============================================================================
/*! Reads a list of source files from the file \a fname
 *
 *  \param fname Name of textfile that contains the loglist
 */
void Log::ReadLoglist(const string& fname)
{
    ifstream in(fname.c_str(), ios::in);
    if (!in) return;

    string str;

    while (getline(in, str, '\n')) {
        int pos = str.find("#");
        str.assign(str, 0, pos);
        RemoveSpaces(str);

        if (!str.empty()) {
            cout << "Adding " << str << " to logging list." << endl;
            m_filetab[str] = 1;
        }
    }

    in.close();
}

//============================================================================
/*! Enables logging to screen. */
void Log::EnableScreen()
{
    m_logscr = true;
}

//============================================================================
/*! Disables logging to screen. */
void Log::DisableScreen()
{
    m_logscr = false;
}

//============================================================================
/*! Logs \a message to file or screen, depending on the current settings.
 *  Logging is only done if the source file is enabled for logging, which is
 *  specified in a text file read by readloglist.
 *
 *  The function takes a variable number of arguments the way printf() does.
 */
void Log::Write(const char message[], ...)
{
    va_list argptr;
    static char s[32000];

    // check logging
    if (m_filetab[m_fname]) {

        va_start (argptr, message);
        vsprintf (s, message, argptr);
        va_end (argptr);

        // open
        Open();

        // log the message
        string str = s;
        str += "   (";
        str += m_fname;
        str += ")\n";
        m_output << str.c_str();
        if (m_logscr) printf(str.c_str());
        if (m_edit) m_edit->insert(str.c_str());

        // close
        Close();
    }
}

//============================================================================
/*! Opens the logfile. */
void Log::Open()
{
    // check create
    if (m_create) {
        // open new log file for writing
        m_output.open("logfile.dat",ios::out);

        // clear create flag
        m_create = false;
    }
    else
        // open existing log file for appending
        m_output.open("logfile.dat",ios::out|ios::app);
}

//============================================================================
/*! Closes the logfile. */
void Log::Close()
{
    // close stream
    m_output.close();
}

#endif
