#
# space element definition file for scmos_n example process
#
# masks:
# cpg - polysilicon interconnect        ccp - contact metal to poly
# caa - active area                     cva - contact metal to metal2
# cmf - metal interconnect              cwn - n-well
# cms - metal2 interconnect             csn - n-channel implant
# cca - contact metal to diffusion      cog - contact to bondpads
#
# See also: maskdata

unit resistance    1     # ohm
unit c_resistance  1e-12 # ohm um^2
unit a_capacitance 1e-6  # aF/um^2
unit e_capacitance 1e-12 # aF/um
unit capacitance   1e-15 # fF

colors :
    cpg   red
    caa   @00cc00
    cmf   @0000cc
    cms   gold
    cca   black
    ccp   black
    cva   black
    cwn   @660066
    csn   glass
    cog   glass

conductors :
  # name    : condition     : mask : resistivity : type
    cond_mf : cmf           : cmf  : 0.045       : m    # first metal
    cond_ms : cms           : cms  : 0.030       : m    # second metal
    cond_pg : cpg           : cpg  : 40          : m    # poly interconnect
    cond_pa : caa !cpg !csn : caa  : 70          : p    # p+ active area
    cond_na : caa !cpg  csn : caa  : 50          : n    # n+ active area

fets :
  # name : condition    : gate d/s
    nenh : cpg caa  csn : cpg  caa     # nenh MOS
    penh : cpg caa !csn : cpg  caa     # penh MOS

contacts :
  # name   : condition        : lay1 lay2 : resistivity
    cont_s : cva cmf cms      : cmf  cms  :   1    # metal to metal2
    cont_p : ccp cmf cpg      : cmf  cpg  : 100    # metal to poly
    cont_a : cca cmf caa !cpg : cmf  caa  : 100    # metal to active area

capacitances :

  # active area capacitances
  # name    :  condition                 : mask1 mask2 : capacitivity
    acap_na :  caa       !cpg  csn !cwn  :  caa @gnd : 100  # n+ bottom
    ecap_na : !caa -caa !-cpg -csn !-cwn : -caa @gnd : 300  # n+ sidewall

    acap_pa :  caa       !cpg  !csn cwn      :  caa @gnd : 500 # p+ bottom
    ecap_pa : !caa -caa !-cpg !-csn cwn -cwn : -caa @gnd : 600 # p+ sidewall

  # polysilicon capacitances
    acap_cpg_sub :  cpg                !caa :  cpg @gnd : 49  # bottom to sub
    ecap_cpg_sub : !cpg -cpg !cmf !cms !caa : -cpg @gnd : 52  # edge to sub

  # first metal capacitances
    acap_cmf_sub :  cmf           !cpg !caa :  cmf @gnd : 25
    ecap_cmf_sub : !cmf -cmf !cms !cpg !caa : -cmf @gnd : 52

    acap_cmf_caa :  cmf      caa !cpg !cca :  cmf caa : 49
    ecap_cmf_caa : !cmf -cmf caa !cms !cpg : -cmf caa : 59

    acap_cmf_cpg :  cmf      cpg !ccp :  cmf  cpg : 49
    ecap_cmf_cpg : !cmf -cmf cpg !cms : -cmf  cpg : 59

  # second metal capacitances
    acap_cms_sub :  cms      !cmf !cpg !caa :  cms @gnd : 16
    ecap_cms_sub : !cms -cms !cmf !cpg !caa : -cms @gnd : 51

    acap_cms_caa :  cms      caa !cmf !cpg :  cms caa : 25
    ecap_cms_caa : !cms -cms caa !cmf !cpg : -cms caa : 54

    acap_cms_cpg :  cms      cpg !cmf :  cms cpg : 25
    ecap_cms_cpg : !cms -cms cpg !cmf : -cms cpg : 54

    acap_cms_cmf :  cms      cmf !cva :  cms cmf : 49
    ecap_cms_cmf : !cms -cms cmf      : -cms cmf : 61

    lcap_cms     : !cms -cms =cms    : -cms =cms : 0.07

#EOF
