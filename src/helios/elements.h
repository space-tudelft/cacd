/*
 * ISC License
 *
 * Copyright (C) 1995-2018 by
 *	Xianfeng Ni
 *	Ulrich Geigenmuller
 *	Simon de Graaf
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

#ifndef elements_h_
#define elements_h_

#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/FileSB.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/SelectioB.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>

extern void glue ();

class genericPara_c {
public:
    virtual void set () = 0;
    virtual void get () = 0;
    virtual void setUdflt () = 0;
//    virtual void setTdflt () = 0;
//    char *readUdfltAsString (FILE *fp);
//    virtual void readUdflt () = 0;
//    virtual void readTdflt () = 0;
//    virtual void writeUdflt () = 0;
//    virtual void addToCmdline () = 0;
//    virtual void addToParafile () = 0;
protected:
    char *keyword;
};

class WtogglePara_c {
// This is meant to contain the part of the code that depends on the
// window system (X-Windows or MS-Windows or whatever)
protected:
        Widget toggle;
        Widget label;
        int getToggleState () {return ((int) XmToggleButtonGetState (toggle));};
        void setToggleState (int state) {XmToggleButtonSetState (toggle, state, TRUE);};
        void nameElement (void* labelWidget, void* toggleWidget) {
               toggle = (Widget) toggleWidget; label = (Widget) labelWidget;};
};

class togglePara_c: public genericPara_c, public WtogglePara_c {
// This class is meant to contain the part of the code that is independent of
// the window system (X-Windows or MS-Windows or whatever).  Stuff depending
// on the window system is hidden in the parent class "WtogglePara_c".
protected:
    int value;
    int udflt;
    int tdflt;
    int gdflt;
public:
    void set () {setToggleState (value);};
    void get () {value = getToggleState ();};
    void setUdflt () {setToggleState ((value = udflt));};
    // The parent class "WtogglePara_c" needs to be pointer to elements (label and toggle)
    // in the window system.  Since these may vary from one window system to another,
    // they are passed as "void*".  The member function "nameElement" of "WtogglePara_c"
    // performs the appropriate type casts (depending on the window system).
    togglePara_c (void *labelName, void *toggleName, int gdflt_in)
        { nameElement (labelName, toggleName); gdflt = gdflt_in;};
};

#endif
