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

// Qt includes
#include <qapplication.h>

#include <stdio.h>
#include <stdlib.h>

using namespace std;

// Project includes
#include "src/spock/src/gui/framework/AppMainWindow.h"

// Qt message handler
void myMessageOutput (QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
#ifndef NO_DEBUG
	fprintf (stderr, "Debug: %s\n", msg);
#endif
	break;
    case QtWarningMsg:
	fprintf (stderr, "Warning: %s\n", msg);
	break;
    default:
	fprintf (stderr, "Fatal: %s\n", msg);
#ifndef NO_DEBUG
	abort();
#else
	exit(1);
#endif
    }
}

int skip_generators = 0;

// Initializes the application
int main (int argc, char **argv)
{
    qInstallMsgHandler (myMessageOutput);
    QApplication a( argc, argv );

    if (argc >= 2 && argv[1][0] == '-' && argv[1][1] == 'g') skip_generators = 1;

    debug("Creating mainwindow...");
    CAppMainWindow myApp(0);
    myApp.resize( 600, 400 );
    myApp.dumpObjectTree();
    a.setMainWidget( &myApp );
    myApp.show();
    return a.exec();
}

#ifdef STATIC
/* libX11.a fix for static linking */
extern "C" void *dlopen (const char *filename, int flag);
void *dlopen (const char *filename, int flag)
{
    return NULL;
}
#endif
