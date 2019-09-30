
unit resistance    1     # ohm
unit vdimension    1e-6  # um
unit a_capacitance 1e-6  # aF/um^2
unit e_capacitance 1e-12 # aF/um

colors :
    cmf   blue
    cms   green
    cog   red

conductors :
  # name    : condition     : mask : resistivity : type
    cond_mf : cmf           : cmf  : 0.045       : m    # first metal
    cond_ms : cms           : cms  : 0.030       : m    # second metal
    cond_cg : cog           : cog  : 0.030       : m    # cmim layer

contacts :
    cont_fs : cmf !cog cva cms : cmf cms : 1    # metal1 to metal2
    cont_cs :      cog cva cms : cog cms : 1    # cmim to metal2

vdimensions :
    ver_cmf        : cmf  : cmf : 1.70 0.70 # 2.4
    ver_cog        : cog  : cog : 2.42 0.28 # 2.7
    ver_cms        : cms  : cms : 2.80 0.70 # 3.5
    omit_cap3d : ver_cmf ver_cog
 #  omit_cap3d : ver_cog ver_cms

dielectrics :
   SiO2   3.9   0.0
   air    1.0   5.0

#EOF
