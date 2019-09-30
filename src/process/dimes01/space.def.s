#
# space element-definition file for DIMES-01 process
#
# masks:
# bi - intrinsic base bi-npn (optional) dp - deep p-well
# bn - burried n-layer                  ic - interconnect
# bs - intrinsic base bs-npn (optional) in - second interconnect
# bw - intrinsic base bw-npn            sn - emitter bs-npn (optional)
# ci - channel p-jfet (optional)        sp - extrinsic base bs-npn (optional)
# co - contact window                   wn - shallow n-layer
# ct - second contact window            wp - extrinsic base bw/bi-npn
# dn - deep n-well
#
# See also: Design manual DIMES-01 process

unit c_resistance  1e-12
unit a_capacitance 1e-03
unit e_capacitance 1e-09

maxkeys 10

new : !dp : epi

colors :
    bn glass
    dp green
    dn magenta
    sp red
    sn red
    bs lightBlue
    bi brown
    ci brown
    cw glass
    wp red
    bw lightBlue
    wn red
    co white
    ic blue
    ct white
    in red

conductors :

  condIC   : ic          : ic  : 0.044 : m
  condIN   : in          : in  : 0.019 : m
  # Epi resistances are 1500, 2700 and 3700 ohm respectively.
  # However, default these resistances are not extracted
  # since they have a bad influence on the performance of
  # Space and since they are usually not so important.
  condEPI  : epi !wp !sp : epi : 0     : n
  condEPI1 : epi wp !sp  : epi : 0     : n	# Epi under WP
  condEPI2 : epi !wp sp  : epi : 0     : n	# Epi under SP
  condBN   : bn          : bn  : 20    : n
  condDP   : dp          : dp  : 8     : p	# deep P-well
  condDN   : dn          : dn  : 4     : n	# deep N-well
  condSP   : sp          : sp  : 25    : p
  condSN   : sn          : sn  : 29    : n
  condBS   : bs !sn      : bs  : 1200  : p
  condBS1  : bs sn       : bs  : 6000  : p
  condBI   : bi !wn      : bi  : 1400  : p
  condBI1  : bi wn       : bi  : 6000  : p
  condCI   : ci !wn      : ci  : 6000  : p
  condCI1  : ci wn       : ci  : 30    : p
  condWP   : wp          : wp  : 25    : p
  condBW   : bw !wn      : bw  : 600   : p
  condBW1  : bw wn       : bw  : 7000  : p
  condWN   : wn          : wn  : 40    : n	# shallow N-layer

fets :

  jfet : wn ci bn : wn ci

bjts :

  npnBS : bn sn bs epi               : ver : sn bs epi
  npnBW : bn wn bw epi               : ver : wn bw epi
  npnBI : bn wn bi epi               : ver : wn bi epi
  pnpWP : bn !wp -wp !bw !bi !ci epi : lat : -wp epi =wp
  pnpSP : bn !sp -sp !bs epi         : lat : -sp epi =sp

connects :

  connBS  : sp bs          : sp bs
  connDN  : dn epi         : dn epi
  connWN  : wn dn          : wn dn
  connBI  : bi wp          : bi wp
  connCI  : ci wp          : ci wp
  connBW  : bw wp          : bw wp
  connDP  : dp wp          : dp wp
  connSN  : sn epi !bs !dn : sn epi
  connSN1 : sn dn          : sn dn
  connBN  : bn epi         : bn epi

contacts :

  contIN : ic ct in  : ic  in :   0
  contSN : ic co sn  : ic  sn : 120
  contSP : ic co sp  : ic  sp :  16
  contWN : ic !co wn : ic  wn :  80
  contWP : ic co wp  : ic  wp : 160
  contDN : ic co dn  : ic  dn :  80

capacitances :

  capBS  : ic  bs !sn             : ic  bs  : 0.109
  capEPI : ic !bn !dp             : ic  epi : 0.123
  capSP  : ic  sp !co !bs         : ic  sp  : 0.119
  capSN  : ic  sn !co             : ic  sn  : 0.077
  capBI  : ic  bi !wn             : ic  bi  : 0.110
  capCI  : ic  ci !wn             : ic  ci  : 0.051
  capWP  : ic  wp !co !bi !bw !ci : ic  wp  : 0.124
  capBW  : ic  bw !wn             : ic  bw  : 0.122
  capDP  : ic  dp !wp !bw         : ic  dp  : 0.123
  capDN  : ic  dn !wn !sn !co     : ic  dn  : 0.081

#EOF
