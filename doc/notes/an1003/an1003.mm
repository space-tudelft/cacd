.T= "Space Green Module Usage"
.DS 2
.rs
.sp 1i
.B
.S 15 20
Space Green Module Usage
.S
.sp 1
.I
S. de Graaf
.sp 1
.R
Circuits and Systems Group
Faculty of Electrical Engineering,
Mathematics and Computer Science
Delft University of Technology
The Netherlands
.DE
.sp 2c
.ce
Report EWI-ENS 10-03
.ce
June 21, 2010
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2010 by the author.

Last revision: June 24, 2010.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
Because the Green module is a blackbox for us,
we want to know more of it.
We know that two spiders must be given to it and that a Green value is calculated for this two.
See also the "Space Green API Implementation Note" in "share/doc/notes/an0202.pdf".
.P
.F+
.PSPIC "an1003/fig0.eps" 5i
.F-
.P
Besides that the spiders needs to have coordinates,
the spiders needs also be connected to a face or an edge data structure
depending on operation mode.
.P
Before we can use function green, we need to init the Green module.
This must be done with a call to function greenInit.
This function needs some global variables and data to work with.
It has only one argument "meters" which specifies the unit (scaling factor) for the spider coordinates.
Function greenInit returns the greenFactor, which is equal to "meters * 4 * PI".
This factor can be used to scale the resulting Green values.
Besides that the constant Epsilon0 is defined (8.855e-12 F/m), which is used
to calculate the capacitance values.
.P
.F+
.PSPIC "an1003/fig1.eps" 5i
.F-
.P
For CAP3D mode the greenCase and greenType must be set to DIEL (= 0).
A global array of dielectric medium data must exist.
The number of dielectric layers is specified with "diel_cnt".
Normally these data comes from a space technology file.
Note that not more than three layers can be specified and
the values of the bottoms must be given in microns.
With the
.P= tecc
technology compiler, however, it is a possible to use more layers
by using the unigreen method.
Then a special technology file is produced with calculated Green data,
but this cost a lot of time to produce.
.br
Function greenInit shall also init a lot of Green parameters,
which default values can be changed by specifying these parameters
in a space parameter file (or on the command line).
For CAP3D, most of these parameters starts with "cap3d.xxx",
where "xxx" is the name of one of the parameters.

.H 1 "WHICH DATA USES THE GREEN MODULE"
It is important to know which data must be given to the Green module.
Nornally the spiders are generated on base of a 3D layout boundary mesh.
The faces are the top, bottom and sidewall faces of the 3D conductors.
The edges are the edges of the faces.
Depending on the chosen mode the data must be filled in correctly.
.H 2 "SPIDER DATA"
The following spider members are used by the Green module:
.fS
    spider -> act_x
    spider -> act_y
    spider -> act_z
    spider -> face
    spider -> edge
    spider -> moments
.fE
First of all, the spider actual x,y,z coordinates.
Second, if the spider is a center spider of a face (PWC mode),
then the spider must point to a face data structure
and not point to a edge data structure (edge = NULL).
If the spider is an edge spider of a face (PWL mode),
then the spider must not point to a face data structure (face = NULL),
but it must point to a face edge data structure.
Third, the spider moments member must be inited to NULL.
This member is set and used by the Green multipole method.
.H 2 "FACE DATA"
The following face members are used by the Green module:
.fS
    face -> corners[0]
    face -> corners[1]
    face -> corners[2]
    face -> corners[3]
    face -> area
    face -> len
.fE
First of all, the face has three or four corner spiders.
The face corners array points to the corner spiders.
Member corners[3] is NULL when the face has only three corners.
For each face the members "area" and "len" must be calculated,
the values can not be zero.
The "len" of a face is the maximum found edge length of that face.
This two values can be calculated from the corner points of the face.

.H 1 "WHICH MODES USES THE GREEN MODULE"
The modes used are dependend of the parameter settings.
.H 2 "PWC VERSUS PWL"
Function green calls eighter function greenPwl or greenPwc depending
on the setting of variable FeModePwl.
This is the main entrance of the Green module:
.fS
double green (spider_t *sp1, spider_t *sp2)
{
    val = FeModePwl ? greenPwl (sp1, sp2) : greenPwc (sp1, sp2);
    if (val < 0) val = 0;
    return (val);
}
.fE
As you see, function green cannot return a negative value.
Thus, the choice is between Piecewise Linear (PWL) or Piecewise Constant (PWC).
Only for be_mode "1c" and "1g" variable FeModePwl is TRUE.
This is set by function greenInit.

.H 2 "PWC MODE"
Piecewise Constant (PWC) collocation (be_mode "0c") is the default Green mode.
Piecewise Constant galerkin mode can be set with "0g".
In that case variable FeModeGalerkin becomes TRUE.
By Piecewise Constant mode the center spiders of the faces must be given to the Green module.
The face "len" is used for making a function choice in combination with the distance of the spiders.
The face "area" is used by CollocationGreenPwc and GalerkinGreenPwc, but only
when the multipole method is not used.
.fS
Private green_t greenPwc (spider_t *sp1, spider_t *sp2)
{
    r = sp1 -> face -> len + sp2 -> face -> len;  /* r > 0 */
    d = spiderDist (sp1, sp2);  /* d >= 0 */

    if (d >= r * pointGreenDist && d > 0) {
	val = PointGreen (sp1, sp2);
    }
    else if (d >= r * collocationGreenDist) {
        val = CollocationGreenPwc (sp1, sp2);
	if (d < r * asymCollocationGreenDist && sp1 != sp2) {
	    val += CollocationGreenPwc (sp2, sp1);
	    val /= 2;
	}
    }
    else {
	val = GalerkinGreenPwc (sp1, sp2);
    }
    return (val);
}
.fE
Variable pointGreenDist is default "infinity", thus function PointGreen is normally not called.
This variable can be set with parameter "cap3d.point_green".
Note that PointGreen is not called when spider sp1 is equal to spider sp2.
.br
Variable collocationGreenDist is default "0", thus function CollocationGreenPwc is always called.
This variable can be set with parameter "cap3d.collocation_green", then
CollocationGreenPwc is only called for d >= r*collocationGreenDist, else
function GalerkinGreenPwc is called.
Note that for equal spiders (d = 0) CollocationGreenPwc is called only for collocationGreenDist is "0".
When FeModeGalerkin is TRUE, then collocationGreenDist is default "infinity" and GalerkinGreenPwc is always called.
.br
Function CollocationGreenPwc is always called twice
when spider sp1 is not equal to spider sp2 and variable asymCollocationGreenDist is "infinity".
This variable can be set with parameter "cap3d.asym_collocation_green".
.P
Note that both functions CollocationGreenPwc and GalerkinGreenPwc shall default call function greenMpole
and return directly (but only if greenMpole returns TRUE).
This because parameter "use_multipoles" is default "on" and useMultiPoles is TRUE.
.fS
Private green_t CollocationGreenPwc (spider_t *spo, spider_t *spc)
{
    if (useMultiPoles && greenMpole (spo, spc, &val)) return val;

    area = spc -> face -> area;
    c = spc -> face -> corners;
    R3Set (opp, spo);
    R3Set (cp1, c[0]);
    R3Set (cp2, c[1]);
    R3Set (cp3, c[2]);
    R3Set (cp4, (c[3] ? c[3] : c[0])); /* triangle or quadrilateral */
    val = doCollocationGreen (opp, cp1, cp2, cp3, cp4);
    return (val / (area * y_stretch));
}
.fE
You see above that CollocationGreenPwc uses the face "area" and variable "y_stretch" and
also the face corner spiders
(this is also the case for function GalerkinGreenPwc).
Variable "y_stretch" is default "1" but can be changed with parameter "cap3d.y_stretch".
Note that this variable is also used by R3Set for the y-coordinate.

.H 2 "MULTIPOLE MODE"
Default, for both collocation and galerkin,
the multipole function greenMpole is called (parameter "use_multipoles" is "on").
For CollocationGreenPwc with unequal spiders it is two times called, because the mean value is used.
Function greenMpole internally uses variable FeModeGalerkin to decide its working.
It calls function getPiePoints, which uses the face corner spiders for PWC (the face area is not used).
For PWL however, the spider edge is used and if the edge points to a face, then
the code tries to go counter-clockwise (using macro ccwa) around the face.
Note that in PWL mode triangular faces are used, which is checked by getPiePoints.
Note that the Green module is not completely self supporting, because macro ccwa calls
function meshCcwAdjacent from the spider module.
Function getPiePoints calls function mediumNumber2 to test of all peripheral points
are laying in the same dielectric medium.
If one point is crossing a dielectric interface, then FALSE is returned.
In that case function greenMpole is returning FALSE and another method must be used.
.P
The multipole method uses some parameters for fine tuning.
.br
For example:
.fS
    multipolesMindist = paramLookupD ("cap3d.mp_min_dist", "2.0");

    oMaxMpOrder = cMaxMpOrder = paramLookupI ("cap3d.mp_max_order", "2");
    defval = mprintf ("%d", oMaxMpOrder);
    oMaxMpOrder = paramLookupI ("multipoles_oMaxMpOrder", defval);
    cMaxMpOrder = paramLookupI ("multipoles_cMaxMpOrder", defval);
.fE

.H 2 "PWL MODE"
Piecewise Linear mode can be set with be_mode "1c" or "1g".
Thus variable FeModePwl becomes TRUE.
By "1g" galerkin is used and variable FeModeGalerkin becomes TRUE.
By Piecewise Linear mode the edge spiders of the faces must be given to the Green module (no center spiders are used).
The edges must point to faces, which must have a triangular shape (three corner points).
The face "len" is not used in this mode.
The face "area" is used by CollocationGreenPwl and GalerkinGreenPwl, but only
when the multipole method is not used.
.fS
Private green_t CollocationGreenPwl (spider_t *spo, spider_t *spc)
{
    if (useMultiPoles && greenMpole (spo, spc, &val)) return (val);

    area = val = 0;
    R3Set (opp, spo);
    for (e = spc -> edge; e; e = NEXT_EDGE (spc, e)) {
        if (!(f = e -> face)) continue; /* sheet conductor */

        sp2 = ccwa (spc, f);
        sp3 = ccwa (sp2, f);
        ASSERT (sp2 != spc && sp2 != sp3 && sp3 != spc);

        R3Set (cp1, spc);
        R3Set (cp2, sp2);
        R3Set (cp3, sp3);
        val  += doCollocationGreen (opp, cp1, cp2, cp3, cp1);
        area += f -> area;
    }
    return (val / (area * y_stretch));
}
.fE
Note that the "area" must be set unequal zero, thus there must be a face "f".
.H 1 "SOME OTHER FUNCTIONS"
.H 2 "ccwa"
This code is only used for Piecewise Linear mode.
.fS
#define ccwa(sp, f) meshCcwAdjacent (sp, f, (face_t *) NULL)

/* Find next spider in ccw (counter-clock wise) order around face.
 */
spider_t *meshCcwAdjacent (spider_t *sp, face_t *face, face_t *newface)
{
    spiderEdge_t *e;

    for (e = sp -> edge; e && e -> face != face; e = NEXT_EDGE (sp, e));

    if (e) {
        if (newface) e -> face = newface;
        return (OTHER_SPIDER (sp, e));
    }
    say ("Internal error %s:%d", __FILE__, __LINE__); die ();
    return (NULL);
}
.fE
.H 2 "NEXT_EDGE"
.fS
#define NEXT_EDGE(sp, e)  (e -> nb)
.fE
.H 2 "OTHER_SPIDER"
.fS
#define OTHER_SPIDER(sp, e)  (e -> oh -> sp)
.fE
.H 2 "PointGreen"
Function PointGreen can only be called when parameter "cap3d.point_green" is set less than infinity.
Note that the face corner coordinates are not used.
.fS
Private green_t PointGreen (spider_t *sp1, spider_t *sp2)
{
    pointR3_t p1, p2;

    R3Set (p1, sp1);
    R3Set (p2, sp2);
    return (greenFunc (&p1, &p2));
}
.fE
.H 2 "R3Set"
This macro places the spider coordinates into a pointR3_t data structure.
.fS
#define R3Set(p, sp) \\
        R3Assign(p, sp->act_x, sp->act_y * y_stretch, sp->act_z)

#define R3Assign(p, a, b, c)   (p.x = a, p.y = b, p.z = c)
.fE

.H 1 "OVERVIEW OF THE GREEN PARAMETERS"
.H 2 "Standard Green Parameters"
.fS
cap3d.be_mode (string) (default: "0c" or "collocation")
.fE
Other values are: "0g" (or "galerkin"), "1c" and "1g".
.br
See the space 3D capacitance extraction manual.
.fS
cap3d.point_green (real > 0) (default: infinity)
.fE
Sets variable pointGreenDist.
See this note.
.br
When set, function PointGreen is called by d >= pointGreenDist.
.fS
cap3d.collocation_green (real) (default: FeModeGalerkin ? infinity : 0)
.fE
Sets variable collocationGreenDist.
See code in this note.
.br
When set, function CollocationGreenPwc/l is called by d >= collocationGreenDist.
.br
When d < collocationGreenDist function GalerkinGreenPwc/l is called instead.
.br
Note that pointGreenDist has higher priority (if set).
.fS
cap3d.asym_collocation_green (real) (default: infinity)
.fE
Sets variable asymCollocationGreenDist.
See code in this note.
.br
When function CollocationGreenPwc/l is called, it is called twice for unequal spiders
and the mean value is used, but only when d < asymCollocationGreenDist.
.fS
cap3d.max_green_terms (integer >= 1) (default: 500)
.fE
See the space 3D capacitance extraction manual.
.fS
cap3d.green_eps (real > 0) (default: 0.001)
.fE
See the space 3D capacitance extraction manual.
.fS
cap3d.col_rel_eps (real > 0) (default: 0.2)
.fE
In Galerkin mode, the "cap3d.green_eps" value multiplied with "cap3d.col_rel_eps" is used.
This is done to get a higher accuracy of collocation.
.H 2 "Debug Parameters"
.fS
debug.print_green_init (bool) (default: off)
.fE
Prints greenInit setup information.
Which Green mode is used (be_mode, FeModeGalerkin, FeModePwl)
The dielectric data which is used and some other settings.
.fS
debug.print_green_terms (bool) (default: off)
.fE
Prints the green terms.
.fS
debug.print_green_gterms (bool) (default: off)
.fE
Prints all the green terms.
.H 2 "Multipole Parameters"
.fS
use_multipoles (bool) (default: on)
.fE
.fS
force_mp_anaint (bool) (default: on)
.fE
When TRUE, enforce use of analytical formulas for inner (collocation) integral
in multipole routine for Green function.
.fS
force_mp_exint (bool) (default: off)
.fE
When TRUE, enforce use of analytical and adaptive numerical integration
in multipole routine for Green function.
.fS
cap3d.mp_min_dist (real) (default: 2.0)
.fE
See space 3D capacitance extraction manual.
.fS
cap3d.mp_max_order (integer 0 ... 3) (default: 2)
.fE
See space 3D capacitance extraction manual.
.fS
multipoles_cMaxMpOrder (default: cap3d.mp_max_order)
multipoles_oMaxMpOrder (default: cap3d.mp_max_order)
.fE
See parameter "cap3d.mp_max_order".
.H 2 "Unigreen Parameters"
.fS
cap3d.use_unigreen (bool) (default: diel_blob_environment ? on : off)
.fE
When there is no diel_blob_environment, it is put off.
A diel_blob_environment exists, when there is a non empty unigreen technology file.
.fS
cap3d.use_unigreen_interpolation (bool) (default: on)
.fE
.H 2 "Integration Parameters"
.fS
use_adptv_intgrtn (bool) (default: off)
.fE
When TRUE, use adaptive integration when Stroud's formulas failed to achieve the desired precision,
in conventional calculation of Green function (galerkin).
.fS
force_adptv_intgrtn (bool) (default: off)
.fE
When TRUE, enforce use of adaptive integration instead of Stroud's formulas,
in conventional calculation of Green function to achieve the desired precision (galerkin).
.H 2 "doCollocationGreen Parameters"
.fS
an_turnover (integer > 0) (default: 100)
.fE
This value is used as argument to functions integrate1D and integrate2D.
This is the maximum number of refinement steps that is tried in sequence
to reach the target accuracy.
It can be used to control running time.
For galerkin a fixed an_turnover of "1000" is used.
.fS
collocation_mode (integer) (default: 0)
.fE
When "1", then numeric collocation is not done for off diagonal images.
.fS
accelerate_levin (bool) (default: off)
.fE
When "on", then "accelerate_levin" is done for numeric collocation.
.H 2 "Special Green Parameters"
.fS
cap3d.use_mean_green_values (bool) (default: off)
.fE
See the Green API note.
.fS
cap3d.merge_images (bool) (default: on)
.fE
See the Green API note.
.fS
cap3d.use_lowest_medium (bool) (default: on)
.fE
See the Green API note.
.fS
cap3d.use_old_images (bool) (default: off)
.fE
See the Green API note.
.fS
cap3d.y_stretch (real > 0) (default: 1.0)
.fE
When set, a spider y-coordinate correction is applied.
See code in this note.
