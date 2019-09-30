# process: ami-c5n (from Jim Plusquellic Aug. 10 2000)

# Wires at distance larger than this in microns have no coupling caps.
lat_cap_window          10.0    # micron

# Used when lateral and non-lateral, e.g. edge coupling caps are
# extracted between conductors. Used in a formula p. 14 of spaceman.ps.
# Default is 1.
compensate_lat_part      1

# Finite element mesh (90-180 degrees). Smaller angles, more accurate.
# Only used when -z option is specified.
max_obtuse		90

# Extraction speed parameter. Only applies for the -z option.
max_delayed		15000

# Threshold between low and high resistivity (Ohms). Used under either -r
# or -z command # line parameter. This must be set to a value larger than
# the sheet resistances of ALL of the conductors otherwise they are NOT
# extracted ! Metal resistance is 0.09.
low_sheet_res		0.01


# ======== None of these are used if -n parameter is specified. ================
# ===============================================================================
# Node eliminated if # of pieces that resistance graph breaks into is less
# than this number. Don't set this to the default value (3) since it causes small
# resistances to be eliminated. Lower, more accurate.
min_art_degree          3

# Node RETAINED if art degree > 1 AND its degree (number of resistances
# connected to it) is >= than this number. Doesn't seem to effect much -- can
# use this to freely lessen the number of components in the circuit. Lower is
# supposed to be more accurate. Default is 4.
min_degree              4

# 2 nodes connected by a resistor less than this value is eliminated if
# its art_degree < min_art_degree and its degree < min_degree and it doesn't
# connect resistances of different types (materials). Lower, more accurate.
# Does NOT effect the total resistance between the nodes. Smallest value
# is 1 !!!
min_res                1      # ohm

# 2 nodes connected by a resistor less than this value is eliminated and
# the nodes are short-circuited. Does effect the total resistance between the
# nodes. Default is 0.
min_sep_res            0.01      # ohm

# Prevents high ohmic shunt paths. If a shunt path resistor/minimum parallel
# resistance path > max_par_res, it is eliminated. Larger values cause fewer
# removals. ratio. I think this also effects total resistance. Yes it does !
max_par_res	100

# Remove all negative resistances. Default is off.
no_neg_res             on


# If for both nodes the coupling cap is connected to, it holds that Ccoup/Cgnd
# is less than min_coup_cap, add coupling cap to gnd cap. Lower values leave more
# coupling caps in the circuit. Default is -infinity. Good value might be 0.1.
min_coup_cap	0.1

# If ground cap value is less than this value, it is removed. Lower values
# leave more substrate caps. Default is 1E-15. Good value might be 1E-17
min_ground_cap	1E-17

# When distributing coupling caps over nodes adjacent to a node that is
# being eliminated. formula - see man page. Lower the value, more detail in
# extracted circuit. A value of 1 gives fast but inaccurate extractions.
frag_coup_cap		0.2

no_neg_cap               on

# These three area parameters are new. Apply only when junction capacitances
# are extracted as area and perimeter elements (like I'm doing now for the
# MOS trans).
min_coup_area            0.04

# Warning -- if this is set to its default value of 1e-11, the as, ad, pd and ps...
# MOS transistor params are not extracted.

min_ground_area          1e-13
frag_coup_area           0.2

# The ratio of L/W is more than this parameter, add an equi-potential node. The
# lower the value, the faster the extraction time. Useful
# when metal resistances are extracted. Default is +infinity.
equi_line_ratio         1.0


# Force flat extraction. Also can use -F on command line.
flat_extraction on

# ===============================================================================

# Either "linear", "non-linear", "area", "area-perimeter" or "separate".
jun_caps "area-perimeter"

#name_ground GND
#name_substrate SUBSTR

# Frequency value: the higher the value, the more nodes that are extracted
# for interconnect - distributed RC effects preserved. Used in conjuction with
# the -G command line option. Example gives 50E9.
# sne.frequency 50E9
