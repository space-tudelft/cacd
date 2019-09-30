#
# space element definition file for ami-c5n process
#       (adapted from scmos_n example process)
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
    cwp   glass
    csn   glass
    csp   glass
    cog   glass

unit resistance    1     # ohm
unit c_resistance  1e-12 # ohm um^2
unit a_capacitance 1e-6  # af/um^2
unit e_capacitance 1e-12 # af/um
unit capacitance   1e-15 # fF
unit vdimension    1e-6  # um
unit shape         1e-6  # um

# Carrier type is important for bipolar devices.
conductors :
  # name    : condition     : mask : resistivity : type
    cond_mf : cmf           : cmf  : 0.09        : m    # first metal
    cond_ms : cms           : cms  : 0.09        : m    # second metal
#    cond_mt : cmt           : cmt  : 0.06        : m    # second metal
    cond_pg : cpg           : cpg  : 25.67       : m    # poly interc.
# Necessary to get the source/drain transistor parameters to extract --
# otherwise tecc complains.
#    cond_pa : caa !cpg !csn : caa  : 102.9       : p    # p+ active area
    cond_pa : caa !cpg !csn : caa  : 0           : p    # p+ active area
#    cond_na : caa !cpg  csn : caa  : 83.2        : n    # n+ active area
    cond_na : caa !cpg  csn : caa  : 0           : n    # n+ active area
    cond_well : cwn         : cwn  : 821.4       : n    # n well

fets :
  # name : condition    : gate d/s : bulk
    nenh : cpg caa  csn : cpg  caa (caa !cpg csn)  : @sub   # nenh MOS
    penh : cpg caa !csn : cpg  caa (caa !cpg !csn) : cwn    # penh MOS

# The actual contact is 2X2 lambda = 0.6*0.6 = 0.36 um^^2. Mosis reports
# the contact resistance but space wants it in ohms*um^^2. For example,
# mosis reports metal to metal2 contact resistance of 1.02. Since
# contact resistance goes down as the contact gets larger, the value
# entered below is 1.02 ohms/contact * 0.36 um^^2 = 0.3672. Space
# computes contact resistance as R = r (ohms*um^^2)/Area. So R is
# computed as 0.3672 ohms um^^2/0.36 um^^2 = 1.02 ohms.

contacts :
  # name   : condition         : lay1 lay2 : resistivity
    cont_s  : cva cms cmf           : cms  cmf  : 0.3672   # metal to metal2
    cont_p  : ccp cmf cpg           : cmf  cpg  : 6.47     # metal to poly
    cont_na : cca cmf caa !cpg csn  : cmf  caa  : 19.11    # metal to ndiff
    cont_pa : cca cmf caa !cpg !csn : cmf  caa  : 38.48    # metal to pdiff

# These are not given in the mosis specs so I've set them to the ndiff
# and pdiff resistances respectively.
    cont_w : cca cmf cwn csn   : cmf  cwn  :  19.11    # metal to well
    cont_b : cca cmf !cwn !csn : cmf  @sub :  38.48    # metal to subs

capacitances :
  # name    :  condition                 : mask1 mask2 : capacitivity
#junction capacitances ndif :
    acap_na :  caa  !cpg  csn !cwn       : @sub caa  : 426.44  # n+ bottom
    ecap_na : !caa !-cpg -csn !-cwn -caa : @sub -caa : 354.44  # n+ sidewall

# Edge cap for nwell is not given by mosis. I estimate it by computing
# the ratio of the edge/area of ndiff above and appling it to the area
# cap of nwell reported by mosis (and given below). e.g. 354.44/426.44*36.67 = 30.5
# The Area cap estimate using same relative fraction as ndiff (30.5*1.20) is
# reasonable illustrating that this edge cap calc is reasonable as well.
#junction capacitances nwell :
    acap_cw :  cwn                  : @sub cwn  : 36.67    # bottom
    ecap_cw : !cwn -cwn             : @sub -cwn : 30.5  # sidewall

#junction capacitances pdif :
    acap_pa :  caa  !cpg  !csn cwn           :  caa cwn : 730 # bottom
    ecap_pa : !caa !-cpg !-csn cwn -cwn -caa : -caa cwn : 251 # sidewall

# Poly area cap to nwell is not given.
  # polysilicon capacitances
    acap_cpg_sub :  cpg                !caa !cwn :  cpg @sub : 88.33
    acap_cpg_cwn :  cpg                !caa  cwn :  cpg cwn  : 88.33

# Mosis does not report edge cap to nwell and substrate for poly.
# Again, I estimate it. Since the edge cap is larger for the other
# conductors (in most cases), I've used the worst case and set it
# equal to area cap.
    ecap_cpg_sub : !cpg -cpg !cmf !cms !caa !cwn : -cpg @sub : 88.33
    ecap_cpg_cwn : !cpg -cpg !cmf !cms !caa  cwn : -cpg cwn  : 88.33

# Again no distinction is given to sub and nwell.
  # first metal capacitances
    acap_cmf_sub :  cmf           !cpg !caa !cwn :  cmf @sub : 39.56
    acap_cmf_cwn :  cmf           !cpg !caa cwn  :  cmf cwn  : 39.56
    ecap_cmf_sub : !cmf -cmf !cms !cpg !caa !cwn : -cmf @sub : 78.33
    ecap_cmf_cwn : !cmf -cmf !cms !cpg !caa cwn  : -cmf cwn  : 78.33

# m1 to ndiff is given. Edge cap not given -- again it is set to worst case.
    acap_cmf_caa :  cmf      caa !cpg !cca !cca :  cmf  caa : 59.33
    ecap_cmf_caa : !cmf -cmf caa !cms !cpg      : -cmf  caa : 59.33

# These are given.
    acap_cmf_cpg :  cmf      cpg !ccp :  cmf  cpg : 53.89
    ecap_cmf_cpg : !cmf -cmf cpg !cms : -cmf  cpg : 65.22

# sub and nwell are not distinguished between.
  # second metal capacitances
    acap_cms_sub :  cms      !cmf !cpg !caa !cwn :  cms @sub : 22.33
    acap_cms_cwn :  cms      !cmf !cpg !caa cwn  :  cms cwn  : 22.33
    ecap_cms_sub : !cms -cms !cmf !cpg !caa !cwn : -cms @sub : 64.33
    ecap_cms_cwn : !cms -cms !cmf !cpg !caa cwn  : -cms cwn  : 64.33

# m2 to ndiff is given. Edge cap not given -- again it is set to worst case.
    acap_cms_caa :  cms      caa !cmf !cpg :  cms caa : 27.0
    ecap_cms_caa : !cms -cms caa !cmf !cpg : -cms caa : 27.0

# These are given.
    acap_cms_cpg :  cms      cpg !cmf :  cms cpg : 22.11
    ecap_cms_cpg : !cms -cms cpg !cmf : -cms cpg : 47.0

# These are given.
    acap_cms_cmf :  cms      cmf !cva :  cms cmf : 33.67
    ecap_cms_cmf : !cms -cms cmf      : -cms cmf : 53.56

#    lcap_cms     : !cms -cms =cms      : -cms =cms : 0.07

#vdimensions :
#    v_caa_on_all : caa !cpg           : caa : 0.30 0.00
#    v_cpg_of_caa : cpg !caa           : cpg : 0.60 0.50
#    v_cpg_on_caa : cpg caa            : cpg : 0.35 0.70
#    v_cmf        : cmf                : cmf : 1.70 0.70
#    v_cms        : cms                : cms : 2.80 0.70
#
#dielectrics :
#   # Dielectric consists of 5 micron thick SiO2
#   # (epsilon = 3.9) on a conducting plane.
#   SiO2   3.9   0.0
#   air    1.0   5.0
#
#sublayers :
#  # name       conductivity  top
#    substrate  6.7           0.0
#
#selfsubres :
#  # resistances to substrate node
#  # area per.  val.  rest
#    1.0  4.0  65851  0.5
#    4.0  8.0  32937  0.5
#    16.0 16.0 16480  0.5
#
#coupsubres:
#  # direct coupling resistances
#  # area1 area2 dist   val.   decr.
#    1.0   1.0  1.0   338990  0.800
#    1.0   1.0  2.0   529316  0.872
#    1.0   1.0  4.0   898693  0.922
#    1.0   1.0  8.0  1628779  0.951

#EOF
