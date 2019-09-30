#
# Space element definition file for MOSIS tsmc025 process
# based on information from
# http://www.mosis.edu/Technical/Testdata/tsmc-025-prm.html
# Technology: SCN025   Vendor: TSMC  Run: N99Y   Feature Size: 0.25 micron
# See also file n99y-params.txt
#
# No data for 3D capacitance extraction available.
# No data for substrate resistance extraction available.
#
# Created on Feb 9, 2000 by Arjan van Genderen, Delft Univ. of Tech., NL
#                        e-mail: A.J.vanGenderen@TUDelft.nl
#

maxkeys 14

colors :
    cpg   red
    caa   green
    cmf   blue
    cms   gold
    cmt   violet
    cmq   aqua
    cmp   lightblue
    cca   black
    ccp   black
    ccc   black
    cva   black
    cvs   black
    cvt   black
    cvq   black
    @sub  pink

unit resistance    1     # resistance in ohm
unit c_resistance  1e-12 # contact resistance in ohm um^2
unit a_capacitance 1e-6  # area capacitance in aF/um^2
unit e_capacitance 1e-12 # edge capacitance in aF/um
unit capacitance   1e-15 # lateral capacitance in fF
unit vdimension    1e-6  # vertical dimensions in um
unit shape         1e-6  # shapes in um

conductors :
  # name    : condition     : mask : resistivity : type
    cond_pg : cpg           : cpg  : 4.1         : m    # poly interconnect
    cond_pa : caa !cpg !csn : caa  : 3.4         : p    # p+ active area
    cond_na : caa !cpg  csn : caa  : 4.7         : n    # n+ active area
    cond_wn : cwn           : cwn  : 0           : n    # n well
  # use the following instead of above for extraction of well resistance
  # (disadvantage: it will take more extraction time).
  # cond_wn : cwn           : cwn  : 1214        : n    # n well
    cond_mf : cmf           : cmf  : 0.08        : m    # first metal
    cond_ms : cms           : cms  : 0.07        : m    # second metal
    cond_mt : cmt           : cmt  : 0.07        : m    # third metal
    cond_mq : cmq           : cmq  : 0.07        : m    # fourth metal
    cond_mp : cmp           : cmp  : 0.03        : m    # fifth metal

fets :
  # name : condition    : gate d/s : bulk
    nenh : cpg caa  csn : cpg  caa : @sub  # nenh MOS
    penh : cpg caa !csn : cpg  caa : cwn   # penh MOS

contacts :
  # name    : condition            : lay1 lay2 : resistivity
    cont_g  : (ccp | ccc) cmf cpg  : cmf  cpg  :  0.334   # metal to poly
    cont_na : (cca | ccc) cmf caa !cpg !cwn csn
                                   : cmf  caa  :  0.397   # metal to n active
    cont_pa : (cca | ccc) cmf caa !cpg cwn !csn
                                   : cmf  caa  :  0.346   # metal to p active
    cont_w : (cca | ccc) cmf cwn csn
                                   : cmf  cwn  :  0       # metal to well
    cont_b : (cca | ccc) cmf !cwn !csn
                                   : cmf  @sub :  0       # metal to substrate
    cont_f : cva cms cmf           : cms  cmf  :  0.118   # metal2 to metal
    cont_s : cvs cmt cms           : cmt  cms  :  0.234   # metal3 to metal2
    cont_t : cvt cmq cmt           : cmq  cmt  :  0.741   # metal4 to metal3
    cont_q : cvq cmp cmq           : cmp  cmq  :  0.975   # metal5 to metal4
  # e.g. for cont_q: resistivity = 7.52 ohm x 0.36 um x 0.36 um = 0.975

capacitances :

    # n well

    acap_nw : cwn                        : cwn @gnd  : 63
    # n active

    acap_na :  caa  !cpg  csn !cwn       : caa  @gnd : 1727  # bottom
    ecap_na : !caa !-cpg -csn !-cwn -caa : -caa @gnd : 417   # sidewall

    # p active

    acap_pa :  caa  !cpg !csn cwn        : caa  @gnd : 1888
    ecap_pa : !caa !-cpg !-csn -cwn -caa : -caa @gnd : 317

    # poly

    acap_cpg_sub :  cpg                !caa :  cpg @gnd : 97
    # poly to n+ act and p+ act are in the spice model

    # metal 1

    acap_cmf_sub :  cmf      !cpg !caa :  cmf @gnd : 37
    ecap_cmf_sub : !cmf -cmf !cpg !caa : -cmf @gnd : 21

    acap_cmf_caa :  cmf      !cpg  caa :  cmf  caa : 50
    # fringe cap. above caa not available.

    acap_cmf_cpg :  cmf      cpg       :  cmf  cpg : 61
    ecap_cmf_cpg : !cmf -cmf cpg       : -cmf  cpg : 67

    # metal 2

    acap_cms_sub :  cms      !cmf !cpg !caa :  cms @gnd : 19
    ecap_cms_sub : !cms -cms !cmf !cpg !caa : -cms @gnd : 57

    acap_cms_caa :  cms      !cmf !cpg  caa :  cms  caa : 20
    # fringe cap. above caa not available.

    acap_cms_cpg :  cms      !cmf cpg       :  cms  cpg : 18
    ecap_cms_cpg : !cms -cms !cmf cpg       : -cms  cpg : 39

    acap_cms_cmf :  cms      cmf            :  cms  cmf : 39
    ecap_cms_cmf : !cms -cms cmf            : -cms  cmf : 49

    # metal 3

    acap_cmt_sub :  cmt      !cms !cmf !cpg !caa :  cmt @gnd : 13
    ecap_cmt_sub : !cmt -cmt !cms !cmf !cpg !caa : -cmt @gnd : 54

    acap_cmt_caa :  cmt      !cms !cmf !cpg  caa :  cmt  caa : 14
    # fringe cap. above caa not available.

    acap_cmt_cpg :  cmt      !cms !cmf cpg       :  cmt  cpg : 10
    ecap_cmt_cpg : !cmt -cmt !cms !cmf cpg       : -cmt  cpg : 29

    acap_cmt_cmf :  cmt      !cms cmf            :  cmt  cmf : 15
    ecap_cmt_cmf : !cmt -cmt !cms cmf            : -cmt  cmf : 33

    acap_cmt_cms :  cmt      cms                 :  cmt  cms : 37
    ecap_cmt_cms : !cmt -cmt cms                 : -cmt  cms : 53

    # metal 4

    acap_cmq_sub :  cmq      !cmt !cms !cmf !cpg !caa :  cmq @gnd : 8
    ecap_cmq_sub : !cmq -cmq !cmt !cms !cmf !cpg !caa : -cmq @gnd : 51

    acap_cmq_caa :  cmq      !cmt !cms !cmf !cpg  caa :  cmq  caa : 11
    # fringe cap. above caa not available.

    acap_cmq_cpg :  cmq      !cmt !cms !cmf cpg       :  cmq  cpg : 7
    ecap_cmq_cpg : !cmq -cmq !cmt !cms !cmf cpg       : -cmq  cpg : 24

    acap_cmq_cmf :  cmq      !cmt !cms cmf            :  cmq  cmf : 9
    ecap_cmq_cmf : !cmq -cmq !cmt !cms cmf            : -cmq  cmf : 27

    acap_cmq_cms :  cmq      !cmt cms                 :  cmq  cms : 15
    ecap_cmq_cms : !cmq -cmq !cmt cms                 : -cmq  cms : 34

    acap_cmq_cmt :  cmq      cmt                      :  cmq  cmt : 38
    ecap_cmq_cmt : !cmq -cmq cmt                      : -cmq  cmt : 53

    # metal 5

    acap_cmp_sub :  cmp      !cmq !cmt !cms !cmf !cpg !caa :  cmp @gnd : 8
    ecap_cmp_sub : !cmp -cmp !cmq !cmt !cms !cmf !cpg !caa : -cmp @gnd : 24

    acap_cmp_caa :  cmp      !cmq !cmt !cms !cmf !cpg  caa :  cmp  caa : 9
    # fringe cap. above caa not available.

    acap_cmp_cpg :  cmp      !cmq !cmt !cms !cmf cpg       :  cmp  cpg : 6
    ecap_cmp_cpg : !cmp -cmp !cmq !cmt !cms !cmf cpg       : -cmp  cpg : 21

    acap_cmp_cmf :  cmp      !cmq !cmt !cms cmf            :  cmp  cmf : 7
    ecap_cmp_cmf : !cmp -cmp !cmq !cmt !cms cmf            : -cmp  cmf : 24

    acap_cmp_cms :  cmp      !cmq !cmt cms                 :  cmp  cms : 9
    ecap_cmp_cms : !cmp -cmp !cmq !cmt cms                 : -cmp  cms : 29

    acap_cmp_cmt :  cmp      !cmq cmt                      :  cmp  cmt : 15
    ecap_cmp_cmt : !cmp -cmp !cmq cmt                      : -cmp  cmt : 35

    acap_cmp_cmq :  cmp      cmq                           :  cmp  cmq : 38
    ecap_cmp_cmq : !cmp -cmp cmq                           : -cmp  cmq : 59

#EOF
