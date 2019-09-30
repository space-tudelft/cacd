# process : pm25
# author  : A.C. de Graaf PICO
# date    : Jan. 3, 1989

# Mask Cross Reference
# Diffusion   D = OD
# Polysilicon P = PS
# Metal       M = IN

conductors :

    cond_D1 :  od !dp !ps !sn                 : od : 350             # p+
    cond_D2 :  od  dp !ps  sn                 : od : 15              # n+
    cond_D3 :  od  dp !ps !sn                 : od : 700             # p-
    cond_P  :          ps                     : ps : 25
    cond_M  :                          in     : in : 0.042


transistors :

    nenh    :  od  dp  ps  sn     !co         : ps od
    penh    :  od !dp  ps !sn     !co         : ps od

contacts :

    cont_D  :   od    !ps          co  in     : in od : 270e-12
    cont_P  :  !od     ps          co  in     : in ps : 90e-12

capacitances:

  # ground capacitances

    cap_M   : !od     !ps              in     : in  : 18e-6
    cap_P   : !od      ps                     : ps  : 22e-6

    cap_D1  :  od  dp !ps  sn                 :  od : 500e-6  # n+/p- bottom
    cap_D2  : !od -od !ps -ps -sn  dp         : -od : 900e-12 # n+/p- along poly
    cap_D3  : !od -od !-ps -sn  dp            : -od : 450e-12 # n+/p- along LOCOS

    cap_D4  :  od !dp !ps !sn                 :  od : 110e-6  # p+/n- bottom
    cap_D5  : !od -od !ps -ps !-sn !dp        : -od : 350e-12 # p+/n- along poly
    cap_D6  : !od -od !-ps !-sn !dp           : -od : 350e-12 # p+/n- along LOCOS

  # coupling capacitances

#    cap_MP1 :          ps              in     : in ps : 530e-6
#    cap_MP2 : !od      ps              in     : in ps : 530e-6
#    cap_MD  :  od     !ps  sn      !co in     : in od : 530e-6
