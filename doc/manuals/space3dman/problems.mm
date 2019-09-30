.H 1 "Solving Problems"
.sY %SE_PROBLEMS% \n(H1
.H 2 "Long Computation Times"
Although
.P= space3d
has been implemented with emphasis on efficient 3D
capacitance extraction methods,
sometimes long extraction times may occur.
This for example happens if too much time is spend on the computation
of irrelevant details.
This is for example the case if the size of the elements is chosen too small,
if the window size is unnecessary large or
if linear shape functions and the Galerkin method are used for too many elements.
A good strategy to circumvent this problem is to first try an extraction
with a parameter set that does not include many details.
Next, a parameter set is used in which more details are included,
and the extraction results are evaluated to inspect the influence
of the parameters.
See also Section %SE_TIMEVSACC%.
.H 2 "Numerical Problems"
If the elastance matrix (see Section %SE_BOUNDARY%) is badly conditioned,
.P= space3d
may be unable to invert this matrix and it may give
error messages like "domain error(s) in sqrt".
One reason for a badly conditioned elastance matrix is
that there is too much difference in element sizes.
A solution in this case is to split the large elements,
either by decreasing the maximum size of the elements
or by adding irregularities to the layout using a symbolic mask.
If very thin conductors are used, the difference between the small vertical
elements and the large horizontal elements may also become too large.
In this case, it may for example be better to specify
a zero thickness for the conductor in the element definition file.
In general, the creation of small elements that are close to large elements,
and the creation of long and narrow elements, should be avoided.
.P
Also the use of the Galerkin method (be_mode 0g or 1g) instead of the
collocation method (be_mode 0c) might help in the above case.
.H 2 "Negative Capacitances"
Some element meshes may also give rise to negative
capacitances.
Negative capacitances may for example occur when conductors
are close to each other and relatively large elements are used.
A typical example is the situation where the bottom of a
transistor gate approaches
its adjacent drain/source regions.
More accurate results (without negative capacitances) are then
obtained by (1) decreasing the maximum size of the elements
and/or (2) increasing the height of the bottom of the gate above the substrate.
If necessary, the parameters
.I min_coup_cap
and/or
.I no_neg_cap
can be set to remove the remaining (small) negative
capacitances.
.H 2 "Mesh Generation Problems"
If
.P= space3d
gives error messages like "mesh.c, 846: assertion failed"
or "refine.c, 507: assertion failed", there is something wrong
with mesh generation.
This problem is often caused by the modeling of
the steps in height above the substrate of the conductors.
If the slope of a conductor near such a transition area is too small,
different transition areas may overlap and the program will not
be able to generate a correct mesh (see Section %SE_VERLIST%
and Section %SE_MESHCON%).
.P
Mesh generation problems may also be caused by the use of eshapes
(see Section %SE_ESHAPES%) and by the fact that, because
of the use of a window (parameter cap3d.be_window),
long and narrow mesh elements may be generated.
Sometimes, problems due to the first cause are solved
by using a resize statement in the element definition file
(see Space User's Manual)
instead of an eshape statement.
