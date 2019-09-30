# directory: demo/suboscil

BEGIN sub3d            # Data for the boundary-element method
be_mode          0g
max_be_area      1     # micron^2
edge_be_ratio    0.01
edge_be_split    0.2
be_window        10    # micron
END sub3d

min_art_degree          3      # Data for network reduction
min_degree              4
min_res               100      # ohm
max_par_res            20  
no_neg_res             on
min_coup_cap            0.05
lat_cap_window          6.0    # micron
max_obtuse            110.0    # degrees
equi_line_ratio         1.0

disp.save_prepass_image  on    # Data for Xspace
