colors:		 #<colors>
    cmf     blue
    cca    black

#<units>
unit vdimension    1e-6     # --> um
unit c_resistance  1e-12    # --> ohm um^2

conductors: #<conductors>
  # name     : boolean : conductor : sheet resistance
    cond_cmf : cmf     : cmf       : 0.060

contacts: #<contacts>
  # name    : boolean condition : pins     : contact resistance
    con_sub : cca cmf !cwn !csn : cmf @sub : 100

capacitances: #<capacitances>
  # name         : boolean cond.  :  pins     : values
    pcap_cmf_sub : cmf            :  cmf @sub : 3.6348e-05
    ecap_cmf_sub : !cmf -cmf      : -cmf @sub : 5e-07 9.3033e-12
                                                1e-06 1.6637e-11
    lcap_cmf_sub : !cmf -cmf =cmf : -cmf =cmf : 5e-07 8.2548e-11
                                                1e-06 4.7931e-11
                                                2e-06 2.556e-11

vdimensions: #<vdimensions>
  # name    : boolean : conductor : zbot thickness
    metal1v : cmf     :   cmf     : 0.95 0.6

dielectrics: #<dielectrics>
  # name       permitivity    zbot
    SiO2         3.9           0
    air          1.0           3.15

sublayers: #<sublayers>
  # name       conductivity   ztop
    epi          6.87          0.0
    substrate    10000        -1.6
