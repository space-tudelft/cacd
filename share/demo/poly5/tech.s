# directory: demo/poly5

unit vdimension  1e-6  # micron

colors :
    cpg  red

conductors :
    resP : cpg : cpg : 0.0

vdimensions :
    dimP : cpg : cpg : 0.5 0.5

dielectrics :
    # Dielectric consists of 5 micron thick SiO2
    # (epsilon = 3.9) on a conducting plane.
    SiO2   3.9   0.0
    air    1.0   5.0

