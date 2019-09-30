# directory: demo/sub3term

BEGIN sub3d
be_mode          0c   # piecewise constant collocation
max_be_area      1.0  # max. size of interior elements in sq. microns
edge_be_ratio    0.05 # max. size edge elem. / max size inter. elem.
edge_be_split    0.2  # split fraction for edge elements
be_window      inf    # infinite window, all resistances
END sub3d

disp.save_prepass_image   on
