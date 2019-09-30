#
# space parameter file for dimes03 process
#

min_art_degree          3
min_degree              4
min_res               100      # ohm
max_par_res            20
no_neg_res             on
min_coup_cap            0.05
lat_cap_window          6.0    # micron
max_obtuse            110.0    # degrees
equi_line_ratio         1.0

BEGIN cap3d                    # Data for 3D capacitance extraction
be_mode                 0c
be_window               2.0
max_be_area             1.0
omit_gate_ds_cap        on
END cap3d

BEGIN sub3d                    # Data for 3D substrate resistance extraction
be_shape                4
be_mode                 0g
max_be_area             1
edge_be_ratio           0.01
edge_be_split           0.2
saw_dist                0
edge_dist               0
be_window              10
END sub3d
