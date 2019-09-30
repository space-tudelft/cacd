# directory: demo/FreePDK45/tabsdemo

BEGIN cap3d            # Data for 3D capacitance extraction
be_mode          0c    # pwc collocation
be_window        0.1   # micron
max_be_area      0.05  # square micron
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

