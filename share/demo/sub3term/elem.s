# directory: demo/sub3term
#
# space element definition file for metal substrate terminals
#

colors :
    cmf   blue
    @sub  pink

conductors :
  # name     : condition : mask : resistivity : type
    cond_cmf : cmf       : cmf  : 0.0         : m

contacts :
  # name     : condition : lay1 lay2 : resistivity
    cont_cmf : cmf       : cmf  @sub : 0.0

sublayers :
  # name       conductivity  top
    epi        6.7           0.0
    substrate  2.0e3        -7.0

#EOF
