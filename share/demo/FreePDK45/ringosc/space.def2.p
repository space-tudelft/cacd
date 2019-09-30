# directory: demo/FreePDK45/ringosc

BEGIN sub3d            # Data for the boundary-element method
be_mode          0g    # pwc galerkin
max_be_area      0.1   # square micron
edge_be_ratio    0.01
edge_be_split    0.2
be_window        1.0   # micron
END sub3d

BEGIN subcap3d         # Data for substrate cap3D extraction
END subcap3d

BEGIN cap3d            # Data for 3D capacitance extraction
be_mode          0c    # pwc collocation
be_window        0.2   # micron
max_be_area      0.01  # square micron
omit_diff_cap     on   # default=on
omit_gate_ds_cap  on   # default=off
omit_gate_gnd_cap on   # default=on
END cap3d

compression           off
min_art_degree          3      # Data for network reduction
min_degree              4
min_res               100      # ohm
min_sep_res            10      # ohm
max_par_res            25

low_sheet_res         1.0    # ohm/square
low_contact_res       1e-13  # ohm.square meter
#no_neg_res             on

min_coup_cap            0.04
lat_cap_window          0.5    # micron
#max_obtuse            110.0    # degrees
equi_line_ratio         1.0

component_coordinates  on
name_substrate      GND
use_corner_nodes    on
equi_line_width     2
low_contact_res  0
low_sheet_res    0

BEGIN disp
#save_prepass_image  on    # Data for Xspace
#draw_sub_term       on
#draw_be_mesh        on
draw_tile            on
draw_fe_mesh         on
draw_out_resistor    on
END disp
