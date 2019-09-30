.nr Hy 0
.SA 1
.nr Ej 1
.nr Fg 0
.H 1 "Line Segment Conversion, the Stateruler"
The conversion of the orthogonal rectangles to line segments
is done based on the stateruler scan algorithm 
given in figure \n(H1.1.
The algorithm is applicable in many situations where the artwork data
is used as input.
E.g. it can equally be used for (layout to circuit) extraction,
design rule checking and the generation of derived
geometries (polygons, groups,
pattern generation).
The complexity of the algorithm is linear
in the number of edges in the layout.
The method is best illustrated with the simple example of
rectangle to vertical line segment conversion.
.\".DS
.fS

begin

    initialize stateruler
    event_status := select_event(event, event_pos);
    stateruler_pos := event_pos;
    while(event_status is not NIL)
    loop
        repeat [make a stateruler profile]
            insert_event(event, stateruler_pos);
            event_status := select_event(event, event_pos);
        until( event_status is NIL or stateruler_pos < event_pos);
        while( stateruler_pos < event_pos) [analyze stateruler]
            stateruler_pos := analyze_stateruler(stateruler_pos);
        stateruler_pos := event_pos;
    end loop
    while(stateruler_pos < MAX_INTEGER) [make final analysis ]
        stateruler_pos := analyze_stateruler(stateruler_pos);
end
.fE
.FG "Stateruler Scan Algorithm" \n(H1.
.\".DE

A stateruler contains a vertical cross section of the layout
description as a sorted list of fields.
The stateruler is made by scanning the layout from left to the right.
Each field has its own state,
determined by a number of variables.
What state variables are used depends on the application the algorithm
is applied to (e.g. design rule verification or circuit extraction).
For rectangle to line segment conversion the state is determined
by the duration, the overlap duration and the checktype.
The duration gives the x_value for which the field will cease to exist.
Likewise the overlap duration gives the x_value for which the
overlap of rectangles in the field will cease to exist.
If there is no overlap this variable is undefined.
The checktype of a stateruler field depends on the checktype of the
rectangles forming the field.
.P
The algorithm proceeds by making and analyzing what we call
stateruler profiles.
A stateruler profile is made by repeatedly selecting an event
and inserting that event in the stateruler.
The selection of events is based upon a selection criterion.
In the case of rectangle to line segment conversion the
events are the rectangles.
The selection criterion is the smallest left value of the rectangle
and if two rectangles have the same left value,
the smallest bottom value.
The procedure 'select_event' returns the event_status.
The event_status will be 'NIL' if there are no more
events to select.
.P
The insertion of an event is done by comparing the bottom
and the top values of the event with
the bottom and top values of the fields in the stateruler,
updating the state of the existing fields and
creating new fields if necessary.
In the case of rectangle to line segment conversion
the state update is done according to a number of rules
derived from the way we want to represent a polygon by
vertical line segments.
Consider figure \n(H1.2a.
The rectangles depicted in this figure must be
converted to the line segments given in figure \n(H1.2b.
.DS
.PS < ../drc/fig2.pic
.PE
.FG "Polygon and line segment representation" \n(H1.
.DE
We distinguish several types of line segments,
characterized by the occurrence type.
They are: 
.BL
.LI
START: A start segment will have the interior of the
polygon located to the right of it.
.LI
STOP: A stop segment will have the interior of the
polygon located to the left of it.
.LI
CHANGE: A change segment indicates a change of checktype.
The polygon area to the right will have the checktype of
the CHANGE segment.
.LI
START_OV: A start_ov segment indicates the start of an overlap of two
areas. The overlap is situated to the right of the segment.
.LI
STOP_OV: A stop_ov segment indicates the cease of an overlap.
The overlap will be situated to the left of this segment.
.LE
.P
Also combinations of the types mentioned above may occur,
so e.g. an segment that is  as well START as CHANGE etc.
.P
Whenever two or more rectangles with different checktypes
start to overlap we will generate a CHANGE segment.
The checktype of such a segment will be made zero.
At the end of the overlap another CHANGE segment is generated,
restoring the checktype.
In doing so we can assure that these overlap areas are
checked because all areas with checktype zero will be
fully checked. (see section 5 ).
.P
Whenever a stateruler has been built the routine 'analyze_stateruler'
is called.
In the case of rectangle to line segment conversion this
routine generates the line segments based upon the state information
present in the individual fields.
The generated segments are characterized by six parameters:
.BL
.LI
x_value: The x_value of the segment.
.LI
occurrence: the occurrence type as described above.
.LI
y_bottom: The y_bottom value of the segment.
.LI
y_top: The y_top value of the segment.
.LI
connection_type: This variable indicates if the segment
has connections to the upper or the lower side or both.
.LI
group_nbr: This variable is an integer indicating what
polygon the segment belongs to.
.LI
checktype: This variable indicates the cell the 
edge belongs to.
.LE
.P
The procedure 'analyze_stateruler' returns the next position
for which the stateruler should be analyzed again.
It returns MAX_INTEGER if that is not necessary.
.P
The final result will be a set of line segment files which
together represent the instanced cell.
These files form the basis for subsequent analysis,
as discussed in the next two sections.
.SP 0
The program performing this conversion is the program makevln.
