#
# space element definition file for DIMES03 process
# initially created by Giuseppe Garcea (?)
# modified by Arjan van Genderen, Feb 11, 2002
#
# masks:
# bn - buried n-layer
# dp - deep p+, device junction isolation
# dn - deep n+, collector plug of all npn
# lp - low ohmic p+ extrinsic base implantation
# lb1 - p-type base link implantation
# lb2 - p type channel implantation (optional)
# co - contact windows
# wn1 - emitter and collector implant
# nwp - p type schottky contact and jfet (optional)
# cap - al2o3 ic-in plate capacitances
# ic - first metal
# ct - contacts to the first metal
# in - second metal
#

maxkeys 13

new : !dp      : epi
new :  co !wn1 : wpX
new :  lb2 co !wp wn1 : glb2  # lb2 gate of fet

conductors:

	condIC	: ic		:ic	: 0.046	: m
	condIN	: in		:in	: 0.019	: m
	condEPI	: epi		:epi	: 2500	: n
	condBN	: bn		:bn	: 35	: n
	condDP	: dp		:dp	: 24.5	: p  #deep P-well
	condDN	: dn		:dn	: 17.5	: n  #deep N-well
	condLP	: epi lp	:lp	: 63	: p  #lp in epi
	condLP1 : dp lp		:lp	: 17.5	: p  #lp in dp
	condLB1	: lb1 epi       :lb1	: 950	: p  #lb1 in epi
	condLB2	: lb2 epi !glb2 :lb2	: 950	: p  #lb2 in epi
	condWN	: wn1		:wn1	: 17.5	: n
	condWP	: wpX    	:wpX	: 17.5	: p

fets:

	pjf : co wn1 lb2 bn     : wn1 lb2

bjts:

      #                                e   b   c
	npn : wn1 lb1 bn epi  : ver : wn1 lb1 epi
        # the pnp is not yet recognized
	pnp :  -lp !lp epi    : lat : -lp epi =lb1

connects:

	connBN_EPI : bn	epi  : bn epi		# connect bn-epi
	connDN_BN  : dn bn   : dn bn		#connect dn-bn
	connLB1_LP : lb1 lp  : lb1 lp		#connect lb1 lp
	connLB2_LP : lb2 lp  : lb2 lp		#connect lb2 lp
	connWN1_DN : wn1 dn  : wn1 dn		#connect wn1-dn
	connWP_LB1 : wpX lb1 : wpX lb1		#connect wp-lb1
	#connWP	: wn1 co : wn1 co		#connect wp-lp
	connWP_LP  : wpX lp  : wpX lp		#connect wp-lp

contacts:

        # res = res_cont_4u_x_4u * 16e-12
	contDP : ic dp		: ic dp   : 800e-12
	contDN : ic !wn1 dn	: ic dn   : 560e-12
	contWN : ic wn1		: ic wn1  : 32e-12
	contLP : ic  !lb1 lp	: ic lp   : 48e-12
	contIN : ic ct !cap in	: ic in   : 800e-15
        connDP_SUB : dp         : dp @sub : 0

junction capacitances pnjun :

      # junction capacitance

        #                           p   n
	acap_epi_sub : epi        : @sub epi  : 0.12
	ecap_dp_epi  : -epi !epi  : @sub -epi : 0.5e-6

	acap_bn_sub  : bn         : @sub bn  : 0.15
	ecap_bn_sub  : -bn !bn    : @sub -bn : 0.2e-6

	acap_lp_bn   : lp bn       : lp bn  : 0.35
	ecap_lp_epi  : -lp !lp epi :-lp epi : 0.6e-6

	acap_lb1_bn  : lb1 bn        : lb1 bn  : 0.28
	ecap_lb1_epi : -lb1 !lb1 epi :-lb1 epi : 0.55e-6

	acap_wp_bn   : wpX !lp bn             : wpX bn  : 0.28
	ecap_wp_epi  : -wpX !wpX !lp !lb1 epi :-wpX epi : 0.55e-6

      # varicaps

	acap_dp_bn   : dp bn : dp bn : 5

capacitances:

      # interconnect capacitance first metal

	acap_ic_epi : ic epi !co !dn !lp !lb1 !wpX : ic epi : 0.18
	acap_ic_dp  : ic dp !co                    : ic dp  : 0.20
	acap_ic_dn  : ic dn !co !wn1               : ic dn  : 0.19
	acap_ic_lp  : ic lp !co !wn1 !wpX          : ic lp  : 0.19
	acap_ic_lb1 : ic lb1 !co !wn1 !wpX         : ic lb1 : 0.17

      # interconnect capacitance second metal

      	acap_in_ic : in !cap !ct ic   : in ic : 0.047 #cap. in-ic
	acap_in_ct : in cap ic ct !co : in ic : 0.38  #cap. in-Al2O3-ic

