.T= "How to Use Xv to Grab a Window"
.DS 2
.rs
.sp 1i
.B
.S 15 20
APPLICATION NOTE
HOW TO USE XV
TO GRAB A WINDOW
.S
.sp 1
.I
S. de Graaf
.sp 1
.R
Circuits and Systems Group
Faculty of Electrical Engineering
Delft University of Technology
The Netherlands
.DE
.sp 2c
.ce
Report ET-CAS 99-04
.ce
November 17, 1999
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 1999 by the author.

Last revision: December 16, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This report describes how to use the program
.P= xv
to grab window pictures.
After the picture is grabbed, it can be edited and stored in certain format
to be used later for some document.
.P
I use the PostScript format and include it with the
.P= psfig
program and
format my input text with "psroff -mm" to get a complete PostScript document.
.P
I explain what you must do with the
.P= xv
program to get a compact PostScript
dump of the window picture.
.P
If you start the
.P= xv
program
.fS
    % xv &
.fE
you get the ``xv main'' window:

.F+
.PSPIC an9904/xv_main.ps 5i
.F-
.H 1 "HOW TO USE XV"
If you have a
.P= xv
main window, click with the right mouse button on this
window to get the ``xv controls'' window:

.F+
.PSPIC an9904/xv_cont.ps 5i
.F-
For grabbing, click with the left mouse button on the [Grab] button.
You get the ``xv grab'' window:

.F+
.PSPIC an9904/xv_grab.ps 5i
.F-
For grabbing a window, click on the [Grab] button and click on the
window you want to grab.
Be sure that this window is totally visible.
You can see the grabbed window in the
.P= xv
main window.
.P
If you want to save this window into a file, click on the [Save] button.
You get the ``xv save'' window:

.F+
.PSPIC an9904/xv_save.ps 5i
.F-
Specify the Format (PostScript) and the Colors (Greyscale) and the Save file
name (xv_grab.ps) and click on [Ok].
You get the ``xv postscript'' window:

.F+
.PSPIC an9904/xv_post.ps 5i
.F-
In the
.P= xv
postscript window you can select the Paper Size (A4) and
adjust the Resolution (100x100dpi) (dpi=dots per inch).
You adjust the
resolution in the Width field with the bottons [<<], [<], [>], [>>].
Click on [Center] to center the bounding box of the window picture.
Click on the [compress] toggle, to choose for compressed PostScript.
Click on [Ok] to save the file.
.P
Add the following lines in your ``mm'' file:
.fS
    .DS
    .F+
    figure xv_grab.ps width 30
    .F-
    .DE
.fE
Use the following command to process/format your ``mm'' file and
use the GhostView program
.P= gv
to preview the results:
.fS
    % psfig howto_xv.mm | psroff -mm > howto_xv.ps
    % gv howto_xv.ps
.fE
.H 1 "GROFF NOTE"
For the GNU
.P= groff
program, we do not use a static display and ``figure'' statement.
.br
We use the following statements:
.fS
    .F+
    .PSPIC xv_grab.ps 5i
    .F-
.fE
