# process scmos-orb-2
#
# space parameter file for scmos_n example process
#
#min_art_degree          3
#min_degree              4
#min_res               100      # ohm
#max_par_res            20
#no_neg_res             on
#min_coup_cap            0.05
#lat_cap_window          6.0    # micron
#max_obtuse            110.0    # degrees
#equi_line_ratio         1.0

# Wires at distance larger than this in microns have no coupling caps.
lat_cap_window          6.0    # micron

# Finite element mesh (90-180 degrees). Smaller angles, more accurate.
#max_obtuse		90

# Extraction speed parameter.
#max_delayed		500

# Threshold between low and high resistivity (Ohms). Involves -r command
# line parameter.
#low_sheet_res		1.0

# Node eliminated if # of pieces that resistance graph breaks into is less
# than this number. Lower, more accurate.
min_art_degree          3

# Node eliminated if art degree is 1 or its degree is less than this number.
# Lower, more accurate.
min_degree              4

# 2 nodes connected by a resistor less than this value is eliminated.
# (plus stuff on min_art_degree and min_degree). Lower, more accurate.
min_res                100      # ohm

# Prevents high ohmic shunt paths. Larger values cause fewer removals. ratio.
max_par_res	20

# If coupling cap divided by either of the substrate caps is less than
# this number, add coupling cap to substrate caps. Lower values leave more
# coupling caps in the circuit. ratio.
min_coup_cap	0.05

# If ground cap value is less than this value, it is removed. Lower values
# leave more substrate caps.
min_ground_cap	0

# When distributing coupling caps over nodes adjacent to a node that is
# being eliminated. formula - see man page.
#frag_coup_cap		0

# If a rectangle's ratio of length/width is greater than this number, output
# a equi-potential node. Higher values may be more accurate ?
#equiv_line_ratio	+infinity
