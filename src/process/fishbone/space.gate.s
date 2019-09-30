# process : c3tu
# author  : N.P. van der Meijs, TU-Delft
# date    : Feb. 21, 1989

maxkeys 14

colors :
    in  blue
    ins yellow
    ps  red
    bb  white
    # od  green
    con black
    cop black
    cps black
    cos black

conductors :

    condIN  : in        : in  : 0.045
    condINS : ins       : ins : 0.030
    condP  : ps         : ps  : 25 	# actually, only if neither sp nor sn.
    condD1 : od !ps sp  : od  : 75 	# p+
    condD2 : od !ps sn  : od  : 55    	# n+

    condDt : od !ps !sn !sp  : od : 65  # terminal

contacts :

    contM  : cos ins in       : ins in : 0.8e-12 # metal to metal2
    contP  : cps in ps        :  in ps : 200e-12 # metal to poly
    contD1 : con in od !ps    :  in od : 300e-12 # metal to n+ diffusion
    contD2 : cop in od !ps    :  in od : 300e-12 # metal to p+ diffusion

capacitances :

  # diffusion capacitances

    # acap_nd : od !ps sn        :  od : 1.9e-4
    # ecap_nd : !od !-ps -od -sn : -od : 310e-12
    # acap_pd : od !ps sp        :  od : 4.5e-4
    # ecap_pd : !od !-ps -od -sp : -od : 570e-12

  # dual plate interconnect capacitances

    # acap_ps_sub :   ps           !od                 :   ps      : 4.9e-05
    # ecap_ps_sub :  -ps           !ins !in  !ps  !od  :  -ps      : 5.4e-11
    # lcap_ps_sub :  -ps  =ps      !ins !in  !ps  !od  :  -ps  =ps : 7.2e-17

    acap_in_sub : !bb   in           !ps  !od            :   in      : 2.5e-05
    ecap_in_sub : !-bb  -in           !ins !in  !ps  !od  :  -in      : 4.5e-11
    lcap_in_sub : !-bb !=bb  -in  =in      !ins !in  !ps  !od  :  -in  =in : 1.1e-16

    acap_in_od : !bb    in        od !ps  !con !cop :   in   od : 4.9e-05
    ecap_in_od : !-bb   -in        od !ins !in  !ps  :  -in   od : 5.4e-11
    lcap_in_od : !-bb !=bb -in  =in   od !ins !in  !ps  :  -in  =in : 9.6e-17

    acap_in_ps : !bb  in        ps !cps      :   in   ps : 4.9e-05
    ecap_in_ps : !-bb -in        ps !ins !in  :  -in   ps : 5.5e-11
    lcap_in_ps : !-bb !=bb -in  =in   ps !ins !in  :  -in  =in : 8.9e-17

    acap_ins_sub : !bb  ins           !in  !ps  !od       :  ins      : 1.3e-05
    ecap_ins_sub : !-bb -ins           !ins !in  !ps  !od  : -ins      : 4.9e-11
    lcap_ins_sub : !-bb !=bb -ins =ins      !ins !in  !ps  !od  : -ins =ins : 2.4e-16

    acap_ins_od : !bb  ins        od !in  !ps       :  ins   od : 1.5e-05
    ecap_ins_od : !-bb -ins        od !ins !in  !ps  : -ins   od : 5.3e-11
    lcap_ins_od : !-bb !=bb -ins =ins   od !ins !in  !ps  : -ins =ins : 2.4e-16

    acap_ins_ps : !bb  ins        ps !od  !in       :  ins   ps : 2.2e-05
    ecap_ins_ps : !-bb -ins        ps !od  !ins !in  : -ins   ps : 6.2e-11
    lcap_ins_ps : !-bb !=bb -ins =ins   ps !od  !ins !in  : -ins =ins : 2.3e-16

    acap_ins_ps_1 : !bb  ins        ps od   !in       :  ins   ps : 1.8e-05
    ecap_ins_ps_1 : !-bb -ins        ps od   !ins !in  : -ins   ps : 5.7e-11
    lcap_ins_ps_1 : !-bb !=bb -ins =ins   ps od   !ins !in  : -ins =ins : 2.3e-16

    acap_ins_in : !bb ins        in !cos :  ins   in : 4.3e-05
    ecap_ins_in : !bb !-bb -ins        in !ins : -ins   in : 8.1e-11
    lcap_ins_in : !bb !=bb -ins =ins   in !ins : -ins =ins : 2.0e-16

  # triple plate interconnect capacitances

    # ecap_in_ps_sub :   in       -ps      !ps  !od  :  -ps      : 2.0e-11
    tcap_in_ps_sub :   !bb in       -ps      !ps  !od  :  -ps   in : 4.5e-11
    # lcap_in_ps_sub :   in       -ps  =ps !ps  !od  :  -ps  =ps : 2.4e-18

    tcap_in_ps_od :   !bb in   od  -ps      !ps  !con !cop :  -ps   in : 3.3e-11

    # ecap_ins_ps_sub :  ins       -ps      !in  !ps  !od  :  -ps      : 3.5e-11
    tcap_ins_ps_sub :  !bb ins       -ps      !in  !ps  !od  :  -ps  ins : 3.8e-11
    # lcap_ins_ps_sub :  ins       -ps  =ps !in  !ps  !od  :  -ps  =ps : 2.6e-17

    # ecap_ins_ps_od :  ins   od  -ps      !in  !ps  :  -ps   od : 1.4e-10
    tcap_ins_ps_od :  !bb ins   od  -ps      !in  !ps  :  -ps  ins : 3.0e-11
    # lcap_ins_ps_od :  ins   od  -ps  =ps !in  !ps  :  -ps  =ps : 1.9e-17

    ecap_ins_in_sub : !-bb ins       -in      !in  !ps  !od  :  -in      : 1.5e-11
    tcap_ins_in_sub : !-bb !bb  ins       -in      !in  !ps  !od  :  -in  ins : 4.8e-11
    lcap_ins_in_sub : !bb !=bb  ins       -in  =in !in  !ps  !od  :  -in  =in : 9.6e-18

    ecap_ins_in_od :  !-bb ins   od  -in      !in  !ps  :  -in   od : 2.4e-11
    tcap_ins_in_od :  !-bb !bb ins   od  -in      !in  !ps  :  -in  ins : 4.3e-11
    lcap_ins_in_od :  !bb !=bb ins   od  -in  =in !in  !ps  :  -in  =in : 7.2e-18

    ecap_ins_in_ps :  !-bb ins   ps  -in      !in  :  -in   ps : 2.1e-11
    tcap_ins_in_ps :  !-bb !bb ins   ps  -in      !in  :  -in  ins : 4.2e-11
    lcap_ins_in_ps :  !-bb !=bb ins   ps  -in  =in !in  :  -in  =in : 2.4e-18

# EOF
