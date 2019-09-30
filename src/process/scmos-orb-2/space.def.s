# process scmos-orb-2
#
# space element definition file for scmos_n example process
#
# masks:
# cpg - polysilicon interconnect        ccp - contact metal to poly
# caa - active area                     cva - contact metal to metal2
# cmf - metal interconnect              cwn - n-well
# cms - metal2 interconnect             csn - n-channel implant
# cca - contact metal to diffusion      cog - contact to bondpads
# csp - p-channel implant               cwp - p-well
#
# See also: maskdata

unit resistance    1     # ohm
unit c_resistance  1e-12 # ohm um^2
unit a_capacitance 1e-6  # aF/um^2
unit e_capacitance 1e-12 # aF/um
unit capacitance   1e-15 # fF

maxkeys 18

colors :
    cpg   red
    caa   green
    cmf   blue
    cms   gold
    cca   black
    ccp   black
    cva   black
    cwn   glass
    csn   glass
    csp   glass
    cog   glass

conductors :
  # name    : condition     : mask : resistivity : type
    cond_mf : cmf           : cmf  : 0.07       : m    # first metal
    cond_ms : cms           : cms  : 0.03       : m    # second metal
    cond_pg : cpg           : cpg  : 24.5       : m    # poly interc.
    cond_pa : caa !cpg !csn : caa  : 61.8       : p    # p+ active area
    cond_na : caa !cpg  csn : caa  : 31.3       : n    # n+ active area
    cond_nw : cwn           : cwn  : 2813       : m    # nwell

fets :
  # name : condition    : gate d/s
    nenh : cpg caa  csn : cpg  caa     # nenh MOS
    penh : cpg caa !csn : cpg  caa     # penh MOS
    # nenh : cpg caa  csn : cpg  caa : @sub    # nenh MOS
    # penh : cpg caa !csn : cpg  caa : cwn     # penh MOS

contacts :
  # name    : condition             : lay1 lay2 : resistivity
    cont_s  : cva cms cmf           : cms  cmf  : 0.64     # metal to metal2
    cont_p  : ccp cmf cpg           : cmf  cpg  : 118.4    # metal to poly
    cont_na : cca cmf caa !cpg csn  : cmf  caa  : 176.0    # metal to ndiff
    cont_pa : cca cmf caa !cpg !csn : cmf  caa  : 576.0    # metal to pdiff

capacitances :

  # active area capacitances
  # name    :  condition                 : mask1 mask2 : capacitivity
    acap_na :  caa  !cpg  csn !cwn       : @gnd caa  : 137  # n+ bottom
    ecap_na : !caa !-cpg -csn !-cwn -caa : @gnd -caa : 480  # n+ sidewall
  #  acap_na :  caa  !cpg  csn !cwn       : @sub caa  : 137  # n+ bottom
  #  ecap_na : !caa !-cpg -csn !-cwn -caa : @sub -caa : 480  # n+ sidewall

    acap_pa :  caa  !cpg  !csn cwn           :  caa @gnd : 315 # p+ bottom
    ecap_pa : !caa !-cpg !-csn cwn -cwn -caa : -caa @gnd : 313 # p+ sidewall
  #  acap_pa :  caa  !cpg  !csn cwn           :  caa @sub : 315 # p+ bottom
  #  ecap_pa : !caa !-cpg !-csn cwn -cwn -caa : -caa @sub : 313 # p+ sidewall

  # polysilicon capacitances
    acap_cpg_sub :  cpg                !caa :  cpg @gnd : 54  # bot  to sub
    ecap_cpg_sub : !cpg -cpg !cmf !cms !caa : -cpg @gnd : 52  # edge to sub

  # first metal capacitances
    acap_cmf_sub :  cmf           !cpg !caa :  cmf @gnd : 23
    ecap_cmf_sub : !cmf -cmf !cms !cpg !caa : -cmf @gnd : 52

    acap_cmf_caa :  cmf      caa !cpg !cca !cca :  cmf  caa : 44
    ecap_cmf_caa : !cmf -cmf caa !cms !cpg      : -cmf  caa : 59

    acap_cmf_cpg :  cmf      cpg !ccp :  cmf  cpg : 41
    ecap_cmf_cpg : !cmf -cmf cpg !cms : -cmf  cpg : 59

  # second metal capacitances
    acap_cms_sub :  cms      !cmf !cpg !caa :  cms @gnd : 14
    ecap_cms_sub : !cms -cms !cmf !cpg !caa : -cms @gnd : 51

    acap_cms_caa :  cms      caa !cmf !cpg :  cms caa : 22
    ecap_cms_caa : !cms -cms caa !cmf !cpg : -cms caa : 54

    acap_cms_cpg :  cms      cpg !cmf :  cms cpg : 19
    ecap_cms_cpg : !cms -cms cpg !cmf : -cms cpg : 54

    acap_cms_cmf :  cms      cmf !cva :  cms cmf : 33
    ecap_cms_cmf : !cms -cms cmf      : -cms cmf : 61

    lcap_cms     : !cms -cms =cms      : -cms =cms : 0.07

#EOF
