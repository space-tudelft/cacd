.T= "Cap3D Extraction Example1"
.de C1
.nf
.S 10
.ft C
..
.de C8
.nf
.S 8
.ft C
..
.de C0
.ft
.S
.fi
..
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE APPLICATION NOTE
ABOUT
CAPACITANCE 3D EXTRACTION
EXAMPLE1 (single2, nLayers=2)
.S
.sp 1
.I
S. de Graaf
.sp 1
.R
Circuits and Systems Group
Faculty of Electrical Engineering
Delft University of Technology
The Netherlands
.DE
.sp 2c
.ce
Report ET-CAS 02-01-1
.ce
April 11, 2002
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2002-2004 by the author.

Last revision: December 10, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "EXAMPLE1 CELL: single2"
.H 2 "single2.ldm"
.C1
:: lambda = 0.01 micron
ms single2
box cmf 0 300 0 600
me
.C0
.F+
.PSPIC an0201/e1_f1.ps 14c
.F-
.F+
.PSPIC an0201/e1_f2.ps 8c
.F-

.H 1 "EXAMPLE1 SETUP"
See project directory:
.C1
    jupiter ../Shadow/hp700.test/src/space/sub3term/GroundedSubstrate
.C0
.H 2 ".dmrc"
.C1
    302
    ./process/scmos_hfcal
    0.01
.C0
.H 2 "space.green.s"
.C1
unit vdimension 1e-6

conductors:
    condIN   : cmf     : cmf  : 0

contacts:
    cont_sub : cmf     : cmf @sub : 0

vdimensions:
    metal1v  : cmf     : cmf  : 250 1

dielectrics:
    SiO2 3.9      0
    air  0.001  250 
.C0
.H 2 "space.green.p"
.C1
lat_cap_window   3000.0 micron
low_sheet_res    0

#min_divergence_term   10
#cap3d.use_old_images           on
#cap3d.use_lowest_medium        off

#cap3d.green_eps 1e-8
cap3d.max_green_terms 100
cap3d.max_be_area               inf
cap3d.be_window                 inf

debug.print_green_init
debug.print_green_terms
debug.print_green_gterms
.C0
.H 2 "execution statement"
.C1
space3d -vC3 -E space.green.t -P space.green.p single2

.C0
Note: There is a big difference between OLD (> 2800 terms) and NEW (206 terms).
.br
See APPENDIX A to C.

.H 1 "EXAMPLE1 SETUP-B"
.H 2 "results"
See APPENDIX D to F.
.H 2 "space.green.s"
.C1
vdimensions:
    metal1v  : cmf     : cmf  : 249 2

dielectrics:
    SiO2 3.9      0
    air  0.001  250 
.C0
.F+
.PSPIC an0201/e1_f3.ps 14c
.F-

.H 1 "APPENDICES"
.SP
.nr Hu 2 \" Level of this .HU heading
.HU "APPENDIX A -- Results: use_old_images=1, min_divergence_term=10"
.C8
-------------------------------------------------------------------------------
Note: No good result is reached for min_divergence_term=3.
      Because false divergence is detected for a number of points (A-E, B-E,
      C-E, D-E, E-E).  Result: cap 457.5016e-18 (SUBSTR, GND);
-------------------------------------------------------------------------------

extracting single2
greenInit: use_lowest_medium=1 use_old_images=1
greenInit: use_mean_green_values=0 merge_images=1
greenInit: use_multipoles=1 test_multipoles=0 min_divergence_term=10
greenInit: be_mode='collocation' FeModeGalerkin=0 FeModePwl=0
greenInit: reading dielectric specification
greenInit: i=1 m='SiO2' e=3.9 b=0
greenInit: i=2 m='air' e=0.001 b=250
greenInit: greenType=0 nLayers=2 maxGreenTerms=100 collocationEps=0.001
strip 0 300 (add)
Schur dimension 6, maxorder 5
greenPwc: d=0 r=4800 sp1=(1200,1200,100200) sp2=(1200,1200,100200) A-A
greenMpole: spo=(1200,1200,100200) spc=(1200,1200,100200) cnt=1 convRadius=1216.55 gs0222
greenMpole( 0,  2): value= 2.901015970 term= 2.901015970 eps=1.000000
greenMpole( 1,  4): value= 1.349738274 term=-1.551277696 eps=1.149317
greenMpole( 2,  6): value= 1.353059303 term= 0.003321029 eps=0.002454
greenMpole( 3,  8): value= 1.351813098 term=-0.001246205 eps=0.000922
greenPwc: CollocationGreenPwc(val= 1.351813098)
greenPwc: d=632.456 r=4800 sp1=(1200,1200,100200) sp2=(600,1200,100400) A-B
greenMpole: spo=(1200,1200,100200) spc=(600,1200,100400) cnt=2 convRadius=1341.64 gs0222
greenMpole( 0,  2): value= 1.262079440 term= 1.262079440 eps=1.000000
greenMpole( 1,  4): value= 0.310238684 term=-0.951840756 eps=3.068092
greenMpole( 2,  6): value= 0.313555266 term= 0.003316583 eps=0.010577
greenMpole( 3,  8): value= 0.312309998 term=-0.001245269 eps=0.003987
greenMpole( 4, 10): value= 0.312974233 term= 0.000664236 eps=0.002122
greenMpole( 5, 12): value= 0.312559153 term=-0.000415080 eps=0.001328
greenMpole( 6, 14): value= 0.312843697 term= 0.000284543 eps=0.000910
greenPwc: CollocationGreenPwc(val= 0.312843697)
greenMpole: spo=(600,1200,100400) spc=(1200,1200,100200) cnt=3 convRadius=1216.55 gs0222
greenMpole( 0,  2): value= 1.149544593 term= 1.149544593 eps=1.000000
greenMpole( 1,  4): value= 0.196428604 term=-0.953115988 eps=4.852226
greenMpole( 2,  6): value= 0.199745196 term= 0.003316591 eps=0.016604
greenMpole( 3,  8): value= 0.198499926 term=-0.001245270 eps=0.006273
greenMpole( 4, 10): value= 0.199164162 term= 0.000664236 eps=0.003335
greenMpole( 5, 12): value= 0.198749082 term=-0.000415080 eps=0.002088
greenMpole( 6, 14): value= 0.199033625 term= 0.000284544 eps=0.001430
greenMpole( 7, 16): value= 0.198826220 term=-0.000207405 eps=0.001043
greenMpole( 8, 18): value= 0.198984180 term= 0.000157960 eps=0.000794
greenPwc: CollocationGreenPwc(val= 0.198984180) val= 0.255913939
greenPwc: d=1341.64 r=3600 sp1=(1200,1200,100200) sp2=(600,2400,100200) A-C
greenMpole: spo=(1200,1200,100200) spc=(600,2400,100200) cnt=4 convRadius=632.456 gs0222
greenMpole( 0,  2): value= 0.727667433 term= 0.727667433 eps=1.000000
greenMpole( 1,  4): value= 0.027890049 term=-0.699777383 eps=25.090575
greenMpole( 2,  6): value= 0.031210992 term= 0.003320943 eps=0.106403
greenMpole( 3,  8): value= 0.029964796 term=-0.001246196 eps=0.041589
greenMpole( 4, 10): value= 0.030629384 term= 0.000664588 eps=0.021698
greenMpole( 5, 12): value= 0.030214132 term=-0.000415252 eps=0.013744
greenMpole( 6, 14): value= 0.030498773 term= 0.000284641 eps=0.009333
greenMpole( 7, 16): value= 0.030291307 term=-0.000207465 eps=0.006849
greenMpole( 8, 18): value= 0.030449307 term= 0.000158000 eps=0.005189
greenMpole( 9, 20): value= 0.030324939 term=-0.000124368 eps=0.004101
greenMpole(10, 22): value= 0.030425392 term= 0.000100453 eps=0.003302
greenMpole(11, 24): value= 0.030342557 term=-0.000082834 eps=0.002730
greenMpole(12, 26): value= 0.030412035 term= 0.000069478 eps=0.002285
greenMpole(13, 28): value= 0.030352925 term=-0.000059110 eps=0.001947
greenMpole(14, 30): value= 0.030403826 term= 0.000050901 eps=0.001674
greenMpole(15, 32): value= 0.030359537 term=-0.000044289 eps=0.001459
greenMpole(16, 34): value= 0.030398422 term= 0.000038885 eps=0.001279
greenMpole(17, 36): value= 0.030364010 term=-0.000034413 eps=0.001133
greenMpole(18, 38): value= 0.030394678 term= 0.000030668 eps=0.001009
greenMpole(19, 40): value= 0.030367175 term=-0.000027503 eps=0.000906
greenPwc: CollocationGreenPwc(val= 0.030367175)
greenMpole: spo=(600,2400,100200) spc=(1200,1200,100200) cnt=5 convRadius=1216.55 gs0222
greenMpole( 0,  2): value= 0.860565423 term= 0.860565423 eps=1.000000
greenMpole( 1,  4): value= 0.066465175 term=-0.794100249 eps=11.947614
greenMpole( 2,  6): value= 0.069786096 term= 0.003320921 eps=0.047587
greenMpole( 3,  8): value= 0.068539903 term=-0.001246193 eps=0.018182
greenMpole( 4, 10): value= 0.069204490 term= 0.000664587 eps=0.009603
greenMpole( 5, 12): value= 0.068789238 term=-0.000415252 eps=0.006037
greenMpole( 6, 14): value= 0.069073879 term= 0.000284641 eps=0.004121
greenMpole( 7, 16): value= 0.068866413 term=-0.000207465 eps=0.003013
greenMpole( 8, 18): value= 0.069024413 term= 0.000158000 eps=0.002289
greenMpole( 9, 20): value= 0.068900045 term=-0.000124368 eps=0.001805
greenMpole(10, 22): value= 0.069000497 term= 0.000100453 eps=0.001456
greenMpole(11, 24): value= 0.068917663 term=-0.000082834 eps=0.001202
greenMpole(12, 26): value= 0.068987141 term= 0.000069478 eps=0.001007
greenMpole(13, 28): value= 0.068928031 term=-0.000059110 eps=0.000858
greenPwc: CollocationGreenPwc(val= 0.068928031) val= 0.049647603
greenPwc: d=1341.64 r=3600 sp1=(1200,1200,100200) sp2=(600,0,100200) A-D (see A-C)
greenMpole: spo=(1200,1200,100200) spc=(600,0,100200) cnt=6 convRadius=632.456 gs0222
greenMpole(19, 40): value= 0.030367175 term=-0.000027503 eps=0.000906
greenPwc: CollocationGreenPwc(val= 0.030367175)
greenMpole: spo=(600,0,100200) spc=(1200,1200,100200) cnt=7 convRadius=1216.55 gs0222
greenMpole(13, 28): value= 0.068928031 term=-0.000059110 eps=0.000858
greenPwc: CollocationGreenPwc(val= 0.068928031) val= 0.049647603
greenPwc: d=632.456 r=4800 sp1=(1200,1200,100200) sp2=(600,1200,100000) A-E
greenMpole: spo=(1200,1200,100200) spc=(600,1200,100000) cnt=8 convRadius=1341.64 gs0222
greenMpole( 0,  2): value= 1.262069480 term= 1.262069480 eps=1.000000
greenMpole( 1,  4): value=-0.001847873 term=-1.263917353 eps=683.984946
greenMpole( 2,  6): value= 0.001477551 term= 0.003325424 eps=2.250633
greenMpole( 3,  8): value= 0.000230414 term=-0.001247136 eps=5.412583
greenMpole( 4, 10): value= 0.000895358 term= 0.000664944 eps=0.742657
greenMpole( 5, 12): value= 0.000479932 term=-0.000415426 eps=0.865592 <----
greenMpole( 6, 14): value= 0.000764671 term= 0.000284739 eps=0.372367
greenMpole( 7, 16): value= 0.000557145 term=-0.000207526 eps=0.372481 <----
greenMpole( 8, 18): value= 0.000715185 term= 0.000158040 eps=0.220978
greenMpole( 9, 20): value= 0.000590789 term=-0.000124396 eps=0.210560
    ...
greenMpole(97,196): value= 0.000645561 term=-0.000001011 eps=0.001567
greenMpole(98,198): value= 0.000646551 term= 0.000000990 eps=0.001532
greenMpole(99,200): value= 0.000645581 term=-0.000000970 eps=0.001502
greenMpole: Greens function truncated after 100 terms (eps=0.00150233)
space3d: Computation of Greens function truncated on max_green_terms (100),
   error specified by green_eps not reached (layers are 'air' and 'air').
greenPwc: CollocationGreenPwc(val= 0.000645581)
greenMpole: spo=(600,1200,100000) spc=(1200,1200,100200) cnt=9 convRadius=1216.55 gs0222
greenMpole( 0,  2): value= 1.149534633 term= 1.149534633 eps=1.000000
greenMpole( 1,  4): value=-0.001905576 term=-1.151440209 eps=604.247718
greenMpole( 2,  6): value= 0.001419856 term= 0.003325432 eps=2.342091
greenMpole( 3,  8): value= 0.000172719 term=-0.001247137 eps=7.220628
greenMpole( 4, 10): value= 0.000837663 term= 0.000664944 eps=0.793809
greenMpole( 5, 12): value= 0.000422237 term=-0.000415426 eps=0.983869 <----
greenMpole( 6, 14): value= 0.000706976 term= 0.000284739 eps=0.402756
greenMpole( 7, 16): value= 0.000499450 term=-0.000207526 eps=0.415510 <----
greenMpole( 8, 18): value= 0.000657490 term= 0.000158040 eps=0.240369
greenMpole( 9, 20): value= 0.000533093 term=-0.000124396 eps=0.233348
    ...
greenMpole(97,196): value= 0.000587865 term=-0.000001011 eps=0.001720
greenMpole(98,198): value= 0.000588856 term= 0.000000990 eps=0.001682
greenMpole(99,200): value= 0.000587886 term=-0.000000970 eps=0.001650
greenMpole: Greens function truncated after 100 terms (eps=0.00164977)
greenPwc: CollocationGreenPwc(val= 0.000587886) val= 0.000616733
greenPwc: d=1200 r=4800 sp1=(1200,1200,100200) sp2=(0,1200,100200) A-F
greenMpole: spo=(1200,1200,100200) spc=(0,1200,100200) cnt=10 convRadius=1216.55 gs0222
greenMpole( 0,  2): value= 0.726788071 term= 0.726788071 eps=1.000000
greenMpole( 1,  4): value= 0.027576922 term=-0.699211149 eps=25.354938
greenMpole( 2,  6): value= 0.030897865 term= 0.003320943 eps=0.107481
greenMpole( 3,  8): value= 0.029651669 term=-0.001246196 eps=0.042028
greenMpole( 4, 10): value= 0.030316257 term= 0.000664588 eps=0.021922
greenMpole( 5, 12): value= 0.029901005 term=-0.000415252 eps=0.013888
greenMpole( 6, 14): value= 0.030185646 term= 0.000284641 eps=0.009430
greenMpole( 7, 16): value= 0.029978180 term=-0.000207465 eps=0.006921
greenMpole( 8, 18): value= 0.030136180 term= 0.000158000 eps=0.005243
greenMpole( 9, 20): value= 0.030011812 term=-0.000124368 eps=0.004144
    ...
greenMpole(18, 38): value= 0.030081551 term= 0.000030668 eps=0.001020
greenMpole(19, 40): value= 0.030054048 term=-0.000027503 eps=0.000915
greenPwc: CollocationGreenPwc(val= 0.030054048)
greenMpole: spo=(0,1200,100200) spc=(1200,1200,100200) cnt=11 convRadius=1216.55 gs0222
greenMpole( 0,  2): value= 0.726788071 term= 0.726788071 eps=1.000000
    ...
greenMpole(19, 40): value= 0.030054048 term=-0.000027503 eps=0.000915
greenPwc: CollocationGreenPwc(val= 0.030054048) val= 0.030054048
greenPwc: d=0 r=4800 sp1=(600,1200,100400) sp2=(600,1200,100400) B-B
greenMpole: spo=(600,1200,100400) spc=(600,1200,100400) cnt=12 convRadius=1341.64 gs0222
greenMpole( 0,  2): value= 2.000069228 term= 2.000069228 eps=1.000000
greenMpole( 1,  4): value= 1.062401806 term=-0.937667423 eps=0.882592
greenMpole( 2,  6): value= 1.065714003 term= 0.003312198 eps=0.003108
greenMpole( 3,  8): value= 1.064469665 term=-0.001244339 eps=0.001169
greenMpole( 4, 10): value= 1.065133547 term= 0.000663882 eps=0.000623
greenPwc: CollocationGreenPwc(val= 1.065133547)
greenPwc: d=1216.55 r=3600 sp1=(600,1200,100400) sp2=(600,2400,100200) B-C
greenMpole: spo=(600,1200,100400) spc=(600,2400,100200) cnt=13 convRadius=632.456 gs0222
greenMpole( 0,  2): value= 0.783838718 term= 0.783838718 eps=1.000000
greenMpole( 1,  4): value= 0.067295952 term=-0.716542766 eps=10.647635
greenMpole( 2,  6): value= 0.070612501 term= 0.003316548 eps=0.046968
greenMpole( 3,  8): value= 0.069367236 term=-0.001245265 eps=0.017952
greenMpole( 4, 10): value= 0.070031471 term= 0.000664235 eps=0.009485
greenMpole( 5, 12): value= 0.069616391 term=-0.000415080 eps=0.005962
greenMpole( 6, 14): value= 0.069900934 term= 0.000284543 eps=0.004071
greenMpole( 7, 16): value= 0.069693529 term=-0.000207405 eps=0.002976
greenMpole( 8, 18): value= 0.069851489 term= 0.000157960 eps=0.002261
greenMpole( 9, 20): value= 0.069727149 term=-0.000124340 eps=0.001783
greenMpole(10, 22): value= 0.069827581 term= 0.000100432 eps=0.001438
greenMpole(11, 24): value= 0.069744762 term=-0.000082819 eps=0.001187
greenMpole(12, 26): value= 0.069814228 term= 0.000069466 eps=0.000995
greenPwc: CollocationGreenPwc(val= 0.069814228)
greenMpole: spo=(600,2400,100200) spc=(600,1200,100400) cnt=14 convRadius=1341.64 gs0222
greenMpole( 0,  2): value= 1.085537994 term= 1.085537994 eps=1.000000
greenMpole( 1,  4): value= 0.268598138 term=-0.816939856 eps=3.041495
greenMpole( 2,  6): value= 0.271914656 term= 0.003316518 eps=0.012197
greenMpole( 3,  8): value= 0.270669395 term=-0.001245261 eps=0.004601
greenMpole( 4, 10): value= 0.271333628 term= 0.000664234 eps=0.002448
greenMpole( 5, 12): value= 0.270918549 term=-0.000415079 eps=0.001532
greenMpole( 6, 14): value= 0.271203092 term= 0.000284543 eps=0.001049
greenMpole( 7, 16): value= 0.270995687 term=-0.000207405 eps=0.000765
greenPwc: CollocationGreenPwc(val= 0.270995687) val= 0.170404958
greenPwc: d=1216.55 r=3600 sp1=(600,1200,100400) sp2=(600,0,100200) B-D (see B-C)
greenMpole: spo=(600,1200,100400) spc=(600,0,100200) cnt=15 convRadius=632.456 gs0222
greenMpole(12, 26): value= 0.069814228 term= 0.000069466 eps=0.000995
greenPwc: CollocationGreenPwc(val= 0.069814228)
greenMpole: spo=(600,0,100200) spc=(600,1200,100400) cnt=16 convRadius=1341.64 gs0222
greenMpole( 7, 16): value= 0.270995687 term=-0.000207405 eps=0.000765
greenPwc: CollocationGreenPwc(val= 0.270995687) val= 0.170404958
greenPwc: d=400 r=4800 sp1=(600,1200,100400) sp2=(600,1200,100000) B-E
greenMpole: spo=(600,1200,100400) spc=(600,1200,100000) cnt=17 convRadius=1341.64 gs0222
greenMpole( 0,  2): value= 1.321896105 term= 1.321896105 eps=1.000000
greenMpole( 1,  4): value=-0.001813485 term=-1.323709590 eps=729.925724
greenMpole( 2,  6): value= 0.001507535 term= 0.003321020 eps=2.202948
greenMpole( 3,  8): value= 0.000261330 term=-0.001246204 eps=4.768697
greenMpole( 4, 10): value= 0.000925921 term= 0.000664590 eps=0.717762
greenMpole( 5, 12): value= 0.000510667 term=-0.000415253 eps=0.813158 <----
greenMpole( 6, 14): value= 0.000795309 term= 0.000284641 eps=0.357900
greenMpole( 7, 16): value= 0.000587843 term=-0.000207466 eps=0.352927
greenMpole( 8, 18): value= 0.000745843 term= 0.000158000 eps=0.211841
    ...
greenMpole(97,196): value= 0.000676235 term=-0.000001011 eps=0.001495
greenMpole(98,198): value= 0.000677225 term= 0.000000990 eps=0.001462
greenMpole(99,200): value= 0.000676255 term=-0.000000970 eps=0.001434
greenMpole: Greens function truncated after 100 terms (eps=0.00143416)
greenPwc: CollocationGreenPwc(val= 0.000676255)
greenMpole: spo=(600,1200,100000) spc=(600,1200,100400) cnt=18 convRadius=1341.64 gs0222
greenMpole( 0,  2): value= 1.321896105 term= 1.321896105 eps=1.000000
greenMpole( 1,  4): value=-0.001813485 term=-1.323709590 eps=729.925724
    ...
greenMpole(99,200): value= 0.000676255 term=-0.000000970 eps=0.001434
greenMpole: Greens function truncated after 100 terms (eps=0.00143416)
greenPwc: CollocationGreenPwc(val= 0.000676255) val= 0.000676255
greenPwc: d=632.456 r=4800 sp1=(600,1200,100400) sp2=(0,1200,100200) B-F (see A-B)
greenMpole: spo=(600,1200,100400) spc=(0,1200,100200) cnt=19 convRadius=1216.55 gs0222
greenMpole( 8, 18): value= 0.198984180 term= 0.000157960 eps=0.000794
greenPwc: CollocationGreenPwc(val= 0.198984180)
greenMpole: spo=(0,1200,100200) spc=(600,1200,100400) cnt=20 convRadius=1341.64 gs0222
greenMpole( 6, 14): value= 0.312843697 term= 0.000284543 eps=0.000910
greenPwc: CollocationGreenPwc(val= 0.312843697) val= 0.255913939
greenPwc: d=0 r=2400 sp1=(600,2400,100200) sp2=(600,2400,100200) C-C
greenMpole: spo=(600,2400,100200) spc=(600,2400,100200) cnt=21 convRadius=632.456 gs0222
greenMpole( 0,  2): value= 4.663004836 term= 4.663004836 eps=1.000000
greenMpole( 1,  4): value= 2.593693285 term=-2.069311551 eps=0.797824
greenMpole( 2,  6): value= 2.597014335 term= 0.003321050 eps=0.001279
greenMpole( 3,  8): value= 2.595768127 term=-0.001246208 eps=0.000480
greenPwc: CollocationGreenPwc(val= 2.595768127)
greenPwc: d=2400 r=2400 sp1=(600,2400,100200) sp2=(600,0,100200) C-D
greenMpole: spo=(600,2400,100200) spc=(600,0,100200) cnt=22 convRadius=632.456 gs0222
greenMpole( 0,  2): value= 0.406854479 term= 0.406854479 eps=1.000000
greenMpole( 1,  4): value= 0.003152371 term=-0.403702109 eps=128.063022
greenMpole( 2,  6): value= 0.006473077 term= 0.003320706 eps=0.513003
greenMpole( 3,  8): value= 0.005226908 term=-0.001246169 eps=0.238414
greenMpole( 4, 10): value= 0.005891489 term= 0.000664581 eps=0.112804
greenMpole( 5, 12): value= 0.005476239 term=-0.000415250 eps=0.075828
greenMpole( 6, 14): value= 0.005760879 term= 0.000284639 eps=0.049409
    ...
greenMpole(40, 82): value= 0.005645292 term= 0.000006126 eps=0.001085
greenMpole(41, 84): value= 0.005639464 term=-0.000005828 eps=0.001033
greenMpole(42, 86): value= 0.005645015 term= 0.000005551 eps=0.000983
greenPwc: CollocationGreenPwc(val= 0.005645015)
greenMpole: spo=(600,0,100200) spc=(600,2400,100200) cnt=23 convRadius=632.456 gs0222
    ...
greenMpole(42, 86): value= 0.005645015 term= 0.000005551 eps=0.000983
greenPwc: CollocationGreenPwc(val= 0.005645015) val= 0.005645015
greenPwc: d=1216.55 r=3600 sp1=(600,2400,100200) sp2=(600,1200,100000) C-E
greenMpole: spo=(600,2400,100200) spc=(600,1200,100000) cnt=24 convRadius=1341.64 gs0222
greenMpole( 0,  2): value= 1.085528035 term= 1.085528035 eps=1.000000
greenMpole( 1,  4): value=-0.001938325 term=-1.087466360 eps=561.034043
greenMpole( 2,  6): value= 0.001387034 term= 0.003325359 eps=2.397461
greenMpole( 3,  8): value= 0.000139905 term=-0.001247129 eps=8.914122
greenMpole( 4, 10): value= 0.000804847 term= 0.000664942 eps=0.826172
greenMpole( 5, 12): value= 0.000389422 term=-0.000415425 eps=1.066774 <----
greenMpole( 6, 14): value= 0.000674160 term= 0.000284738 eps=0.422360
greenMpole( 7, 16): value= 0.000466634 term=-0.000207526 eps=0.444729 <----
greenMpole( 8, 18): value= 0.000624674 term= 0.000158040 eps=0.252996
greenMpole( 9, 20): value= 0.000500278 term=-0.000124396 eps=0.248654
    ...
greenMpole(97,196): value= 0.000555050 term=-0.000001011 eps=0.001822
greenMpole(98,198): value= 0.000556040 term= 0.000000990 eps=0.001781
greenMpole(99,200): value= 0.000555070 term=-0.000000970 eps=0.001747
greenMpole: Greens function truncated after 100 terms (eps=0.0017473)
greenPwc: CollocationGreenPwc(val= 0.000555070)
greenMpole: spo=(600,1200,100000) spc=(600,2400,100200) cnt=25 convRadius=632.456 gs0222
greenMpole( 0,  2): value= 0.783828759 term= 0.783828759 eps=1.000000
greenMpole( 1,  4): value=-0.002093031 term=-0.785921789 eps=375.494648
greenMpole( 2,  6): value= 0.001232359 term= 0.003325389 eps=2.698394
greenMpole( 3,  8): value=-0.000014774 term=-0.001247132 eps=84.415441 <----
greenMpole( 4, 10): value= 0.000650169 term= 0.000664943 eps=1.022723
greenMpole( 5, 12): value= 0.000234744 term=-0.000415425 eps=1.769697 <----
greenMpole( 6, 14): value= 0.000519482 term= 0.000284738 eps=0.548120
greenMpole( 7, 16): value= 0.000311956 term=-0.000207526 eps=0.665241 <----
greenMpole( 8, 18): value= 0.000469996 term= 0.000158040 eps=0.336258
greenMpole( 9, 20): value= 0.000345600 term=-0.000124396 eps=0.359943 <----
greenMpole: Greens function divergence after 10 terms (eps=0.336258)
greenPwc: CollocationGreenPwc(val= 0.000345600) val= 0.000450335
greenPwc: d=1341.64 r=3600 sp1=(600,2400,100200) sp2=(0,1200,100200) C-F (see A-C)
greenMpole: spo=(600,2400,100200) spc=(0,1200,100200) cnt=26 convRadius=1216.55 gs0222
greenMpole(13, 28): value= 0.068928031 term=-0.000059110 eps=0.000858
greenPwc: CollocationGreenPwc(val= 0.068928031)
greenMpole: spo=(0,1200,100200) spc=(600,2400,100200) cnt=27 convRadius=632.456 gs0222
greenMpole(19, 40): value= 0.030367175 term=-0.000027503 eps=0.000906
greenPwc: CollocationGreenPwc(val= 0.030367175) val= 0.049647603
greenPwc: d=0 r=2400 sp1=(600,0,100200) sp2=(600,0,100200) D-D (see C-C)
greenMpole: spo=(600,0,100200) spc=(600,0,100200) cnt=28 convRadius=632.456 gs0222
greenMpole( 3,  8): value= 2.595768127 term=-0.001246208 eps=0.000480
greenPwc: CollocationGreenPwc(val= 2.595768127)
greenPwc: d=1216.55 r=3600 sp1=(600,0,100200) sp2=(600,1200,100000) D-E (see C-E)
greenMpole: spo=(600,0,100200) spc=(600,1200,100000) cnt=29 convRadius=1341.64 gs0222
greenMpole(99,200): value= 0.000555070 term=-0.000000970 eps=0.001747
greenMpole: Greens function truncated after 100 terms (eps=0.0017473)
greenPwc: CollocationGreenPwc(val= 0.000555070)
greenMpole: spo=(600,1200,100000) spc=(600,0,100200) cnt=30 convRadius=632.456 gs0222
greenMpole( 9, 20): value= 0.000345600 term=-0.000124396 eps=0.359943
greenMpole: Greens function \fB\s+1divergence\s0\fP after 10 terms (eps=0.336258)
greenPwc: CollocationGreenPwc(val= 0.000345600) val= 0.000450335
greenPwc: d=1341.64 r=3600 sp1=(600,0,100200) sp2=(0,1200,100200) D-F (see C-F)
greenMpole: spo=(600,0,100200) spc=(0,1200,100200) cnt=31 convRadius=1216.55 gs0222
greenMpole(13, 28): value= 0.068928031 term=-0.000059110 eps=0.000858
greenPwc: CollocationGreenPwc(val= 0.068928031)
greenMpole: spo=(0,1200,100200) spc=(600,0,100200) cnt=32 convRadius=632.456 gs0222
greenMpole(19, 40): value= 0.030367175 term=-0.000027503 eps=0.000906
greenPwc: CollocationGreenPwc(val= 0.030367175) val= 0.049647603
greenPwc: d=0 r=4800 sp1=(600,1200,100000) sp2=(600,1200,100000) E-E
greenMpole: spo=(600,1200,100000) spc=(600,1200,100000) cnt=33 convRadius=1341.64 gs0222
greenMpole( 0,  2): value= 2.000049309 term= 2.000049309 eps=1.000000
greenMpole( 1,  4): value=-0.001473282 term=-2.001522591 eps=1358.546744
greenMpole( 2,  6): value= 0.001856598 term= 0.003329880 eps=1.793539
greenMpole( 3,  8): value= 0.000608524 term=-0.001248074 eps=2.050987 <----
greenMpole( 4, 10): value= 0.001273823 term= 0.000665299 eps=0.522286
greenMpole( 5, 12): value= 0.000858224 term=-0.000415599 eps=0.484255
greenMpole( 6, 14): value= 0.001143060 term= 0.000284836 eps=0.249187
greenMpole( 7, 16): value= 0.000935474 term=-0.000207587 eps=0.221905
greenMpole( 8, 18): value= 0.001093554 term= 0.000158080 eps=0.144556
    ...
greenMpole(95,192): value= 0.001023892 term=-0.000001055 eps=0.001031
greenMpole(96,194): value= 0.001024925 term= 0.000001033 eps=0.001008
greenMpole(97,196): value= 0.001023913 term=-0.000001011 eps=0.000988
greenPwc: CollocationGreenPwc(val= 0.001023913)
greenPwc: d=632.456 r=4800 sp1=(600,1200,100000) sp2=(0,1200,100200) E-F (see A-E)
greenMpole: spo=(600,1200,100000) spc=(0,1200,100200) cnt=34 convRadius=1216.55 gs0222
greenMpole(99,200): value= 0.000587886 term=-0.000000970 eps=0.001650
greenMpole: Greens function truncated after 100 terms (eps=0.00164977)
greenPwc: CollocationGreenPwc(val= 0.000587886)
greenMpole: spo=(0,1200,100200) spc=(600,1200,100000) cnt=35 convRadius=1341.64 gs0222
greenMpole(99,200): value= 0.000645581 term=-0.000000970 eps=0.001502
greenMpole: Greens function truncated after 100 terms (eps=0.00150233)
greenPwc: CollocationGreenPwc(val= 0.000645581) val= 0.000616733
greenPwc: d=0 r=4800 sp1=(0,1200,100200) sp2=(0,1200,100200) F-F (see A-A)
greenMpole: spo=(0,1200,100200) spc=(0,1200,100200) cnt=36 convRadius=1216.55 gs0222
greenMpole( 3,  8): value= 1.351813098 term=-0.001246205 eps=0.000922
greenPwc: CollocationGreenPwc(val= 1.351813098)
space3d: Warning: maximum error not reached for 27.8% of the Greens functions.

overall resource utilization:
        memory allocation  : 0.184 Mbyte
        user time          :         0.1
        system time        :         0.6
-------------------------------------------------------------------------------

/* Date: 9-Apr-02 15:33:57 GMT */
network single2 ()
{
    cap 271.8203e-18 (SUBSTR, GND);
}
-------------------------------------------------------------------------------
By min_divergence_term=30

    cap 271.8146e-18 (SUBSTR, GND);
-------------------------------------------------------------------------------
.C0

.HU "APPENDIX B -- Results: use_old_images=0"
.C8
greenPwc: d=0 r=4800 sp1=(1200,1200,100200) sp2=(1200,1200,100200) A-A
greenMpole: spo=(1200,1200,100200) spc=(1200,1200,100200) cnt=1 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 1.352229489 term= 1.352229489 eps=1.000000
greenMpole( 1,  5): value= 1.352228212 term=-0.000001277 eps=0.000001
greenPwc: CollocationGreenPwc(val= 1.352228212)
greenPwc: d=632.456 r=4800 sp1=(1200,1200,100200) sp2=(600,1200,100400) A-B
greenMpole: spo=(1200,1200,100200) spc=(600,1200,100400) cnt=2 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.312726145 term= 0.312726145 eps=1.000000
greenMpole( 1,  5): value= 0.312724870 term=-0.000001275 eps=0.000004
greenPwc: CollocationGreenPwc(val= 0.312724870)
greenMpole: spo=(600,1200,100400) spc=(1200,1200,100200) cnt=3 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 0.198916074 term= 0.198916074 eps=1.000000
greenMpole( 1,  5): value= 0.198914798 term=-0.000001275 eps=0.000006
greenPwc: CollocationGreenPwc(val= 0.198914798) val= 0.255819834
greenPwc: d=1341.64 r=3600 sp1=(1200,1200,100200) sp2=(600,2400,100200) A-C
greenMpole: spo=(1200,1200,100200) spc=(600,2400,100200) cnt=4 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 0.030381186 term= 0.030381186 eps=1.000000
greenMpole( 1,  5): value= 0.030379909 term=-0.000001277 eps=0.000042
greenPwc: CollocationGreenPwc(val= 0.030379909)
greenMpole: spo=(600,2400,100200) spc=(1200,1200,100200) cnt=5 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 0.068956292 term= 0.068956292 eps=1.000000
greenMpole( 1,  5): value= 0.068955015 term=-0.000001277 eps=0.000019
greenPwc: CollocationGreenPwc(val= 0.068955015) val= 0.049667462
greenPwc: d=1341.64 r=3600 sp1=(1200,1200,100200) sp2=(600,0,100200) A-D
greenMpole: spo=(1200,1200,100200) spc=(600,0,100200) cnt=6 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 0.030381186 term= 0.030381186 eps=1.000000
greenMpole( 1,  5): value= 0.030379909 term=-0.000001277 eps=0.000042
greenPwc: CollocationGreenPwc(val= 0.030379909)
greenMpole: spo=(600,0,100200) spc=(1200,1200,100200) cnt=7 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 0.068956292 term= 0.068956292 eps=1.000000
greenMpole( 1,  5): value= 0.068955015 term=-0.000001277 eps=0.000019
greenPwc: CollocationGreenPwc(val= 0.068955015) val= 0.049667462
greenPwc: d=632.456 r=4800 sp1=(1200,1200,100200) sp2=(600,1200,100000) A-E
greenMpole: spo=(1200,1200,100200) spc=(600,1200,100000) cnt=8 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.000647049 term= 0.000647049 eps=1.000000
greenMpole( 1,  5): value= 0.000645770 term=-0.000001279 eps=0.001981
greenMpole( 2,  7): value= 0.000646197 term= 0.000000426 eps=0.000660
greenPwc: CollocationGreenPwc(val= 0.000646197)
greenMpole: spo=(600,1200,100000) spc=(1200,1200,100200) cnt=9 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 0.000589354 term= 0.000589354 eps=1.000000
greenMpole( 1,  5): value= 0.000588075 term=-0.000001279 eps=0.002175
greenMpole( 2,  7): value= 0.000588501 term= 0.000000426 eps=0.000725
greenPwc: CollocationGreenPwc(val= 0.000588501) val= 0.000617349
greenPwc: d=1200 r=4800 sp1=(1200,1200,100200) sp2=(0,1200,100200) A-F
greenMpole: spo=(1200,1200,100200) spc=(0,1200,100200) cnt=10 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 0.030068059 term= 0.030068059 eps=1.000000
greenMpole( 1,  5): value= 0.030066782 term=-0.000001277 eps=0.000042
greenPwc: CollocationGreenPwc(val= 0.030066782)
greenMpole: spo=(0,1200,100200) spc=(1200,1200,100200) cnt=11 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 0.030068059 term= 0.030068059 eps=1.000000
greenMpole( 1,  5): value= 0.030066782 term=-0.000001277 eps=0.000042
greenPwc: CollocationGreenPwc(val= 0.030066782) val= 0.030066782
greenPwc: d=0 r=4800 sp1=(600,1200,100400) sp2=(600,1200,100400) B-B
greenMpole: spo=(600,1200,100400) spc=(600,1200,100400) cnt=12 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 1.064885569 term= 1.064885569 eps=1.000000
greenMpole( 1,  5): value= 1.064884295 term=-0.000001273 eps=0.000001
greenPwc: CollocationGreenPwc(val= 1.064884295)
greenPwc: d=1216.55 r=3600 sp1=(600,1200,100400) sp2=(600,2400,100200) B-C
greenMpole: spo=(600,1200,100400) spc=(600,2400,100200) cnt=13 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 0.069783382 term= 0.069783382 eps=1.000000
greenMpole( 1,  5): value= 0.069782107 term=-0.000001275 eps=0.000018
greenPwc: CollocationGreenPwc(val= 0.069782107)
greenMpole: spo=(600,2400,100200) spc=(600,1200,100400) cnt=14 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.271085541 term= 0.271085541 eps=1.000000
greenMpole( 1,  5): value= 0.271084265 term=-0.000001275 eps=0.000005
greenPwc: CollocationGreenPwc(val= 0.271084265) val= 0.170433186
greenPwc: d=1216.55 r=3600 sp1=(600,1200,100400) sp2=(600,0,100200) B-D
greenMpole: spo=(600,1200,100400) spc=(600,0,100200) cnt=15 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 0.069783382 term= 0.069783382 eps=1.000000
greenMpole( 1,  5): value= 0.069782107 term=-0.000001275 eps=0.000018
greenPwc: CollocationGreenPwc(val= 0.069782107)
greenMpole: spo=(600,0,100200) spc=(600,1200,100400) cnt=16 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.271085541 term= 0.271085541 eps=1.000000
greenMpole( 1,  5): value= 0.271084265 term=-0.000001275 eps=0.000005
greenPwc: CollocationGreenPwc(val= 0.271084265) val= 0.170433186
greenPwc: d=400 r=4800 sp1=(600,1200,100400) sp2=(600,1200,100000) B-E
greenMpole: spo=(600,1200,100400) spc=(600,1200,100000) cnt=17 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.000677722 term= 0.000677722 eps=1.000000
greenMpole( 1,  5): value= 0.000676444 term=-0.000001277 eps=0.001888
greenMpole( 2,  7): value= 0.000676871 term= 0.000000426 eps=0.000630
greenPwc: CollocationGreenPwc(val= 0.000676871)
greenMpole: spo=(600,1200,100000) spc=(600,1200,100400) cnt=18 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.000677722 term= 0.000677722 eps=1.000000
greenMpole( 1,  5): value= 0.000676444 term=-0.000001277 eps=0.001888
greenMpole( 2,  7): value= 0.000676871 term= 0.000000426 eps=0.000630
greenPwc: CollocationGreenPwc(val= 0.000676871) val= 0.000676871
greenPwc: d=632.456 r=4800 sp1=(600,1200,100400) sp2=(0,1200,100200) B-F
greenMpole: spo=(600,1200,100400) spc=(0,1200,100200) cnt=19 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 0.198916074 term= 0.198916074 eps=1.000000
greenMpole( 1,  5): value= 0.198914798 term=-0.000001275 eps=0.000006
greenPwc: CollocationGreenPwc(val= 0.198914798)
greenMpole: spo=(0,1200,100200) spc=(600,1200,100400) cnt=20 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.312726145 term= 0.312726145 eps=1.000000
greenMpole( 1,  5): value= 0.312724870 term=-0.000001275 eps=0.000004
greenPwc: CollocationGreenPwc(val= 0.312724870) val= 0.255819834
greenPwc: d=0 r=2400 sp1=(600,2400,100200) sp2=(600,2400,100200) C-C
greenMpole: spo=(600,2400,100200) spc=(600,2400,100200) cnt=21 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 2.596184519 term= 2.596184519 eps=1.000000
greenMpole( 1,  5): value= 2.596183242 term=-0.000001277 eps=0.000000
greenPwc: CollocationGreenPwc(val= 2.596183242)
greenPwc: d=2400 r=2400 sp1=(600,2400,100200) sp2=(600,0,100200) C-D
greenMpole: spo=(600,2400,100200) spc=(600,0,100200) cnt=22 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 0.005643292 term= 0.005643292 eps=1.000000
greenMpole( 1,  5): value= 0.005642015 term=-0.000001277 eps=0.000226
greenPwc: CollocationGreenPwc(val= 0.005642015)
greenMpole: spo=(600,0,100200) spc=(600,2400,100200) cnt=23 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 0.005643292 term= 0.005643292 eps=1.000000
greenMpole( 1,  5): value= 0.005642015 term=-0.000001277 eps=0.000226
greenPwc: CollocationGreenPwc(val= 0.005642015) val= 0.005642015
greenPwc: d=1216.55 r=3600 sp1=(600,2400,100200) sp2=(600,1200,100000) C-E
greenMpole: spo=(600,2400,100200) spc=(600,1200,100000) cnt=24 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.000556538 term= 0.000556538 eps=1.000000
greenMpole( 1,  5): value= 0.000555259 term=-0.000001279 eps=0.002304
greenMpole( 2,  7): value= 0.000555686 term= 0.000000426 eps=0.000767
greenPwc: CollocationGreenPwc(val= 0.000555686)
greenMpole: spo=(600,1200,100000) spc=(600,2400,100200) cnt=25 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 0.000401860 term= 0.000401860 eps=1.000000
greenMpole( 1,  5): value= 0.000400581 term=-0.000001279 eps=0.003193
greenMpole( 2,  7): value= 0.000401008 term= 0.000000426 eps=0.001063
greenMpole( 3,  9): value= 0.000400795 term=-0.000000213 eps=0.000532
greenPwc: CollocationGreenPwc(val= 0.000400795) val= 0.000478240
greenPwc: d=1341.64 r=3600 sp1=(600,2400,100200) sp2=(0,1200,100200) C-F
greenMpole: spo=(600,2400,100200) spc=(0,1200,100200) cnt=26 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 0.068956292 term= 0.068956292 eps=1.000000
greenMpole( 1,  5): value= 0.068955015 term=-0.000001277 eps=0.000019
greenPwc: CollocationGreenPwc(val= 0.068955015)
greenMpole: spo=(0,1200,100200) spc=(600,2400,100200) cnt=27 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 0.030381186 term= 0.030381186 eps=1.000000
greenMpole( 1,  5): value= 0.030379909 term=-0.000001277 eps=0.000042
greenPwc: CollocationGreenPwc(val= 0.030379909) val= 0.049667462
greenPwc: d=0 r=2400 sp1=(600,0,100200) sp2=(600,0,100200) D-D
greenMpole: spo=(600,0,100200) spc=(600,0,100200) cnt=28 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 2.596184519 term= 2.596184519 eps=1.000000
greenMpole( 1,  5): value= 2.596183242 term=-0.000001277 eps=0.000000
greenPwc: CollocationGreenPwc(val= 2.596183242)
greenPwc: d=1216.55 r=3600 sp1=(600,0,100200) sp2=(600,1200,100000) D-E
greenMpole: spo=(600,0,100200) spc=(600,1200,100000) cnt=29 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.000556538 term= 0.000556538 eps=1.000000
greenMpole( 1,  5): value= 0.000555259 term=-0.000001279 eps=0.002304
greenMpole( 2,  7): value= 0.000555686 term= 0.000000426 eps=0.000767
greenPwc: CollocationGreenPwc(val= 0.000555686)
greenMpole: spo=(600,1200,100000) spc=(600,0,100200) cnt=30 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 0.000401860 term= 0.000401860 eps=1.000000
greenMpole( 1,  5): value= 0.000400581 term=-0.000001279 eps=0.003193
greenMpole( 2,  7): value= 0.000401008 term= 0.000000426 eps=0.001063
greenMpole( 3,  9): value= 0.000400795 term=-0.000000213 eps=0.000532
greenPwc: CollocationGreenPwc(val= 0.000400795) val= 0.000478240
greenPwc: d=1341.64 r=3600 sp1=(600,0,100200) sp2=(0,1200,100200) D-F
greenMpole: spo=(600,0,100200) spc=(0,1200,100200) cnt=31 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 0.068956292 term= 0.068956292 eps=1.000000
greenMpole( 1,  5): value= 0.068955015 term=-0.000001277 eps=0.000019
greenPwc: CollocationGreenPwc(val= 0.068955015)
greenMpole: spo=(0,1200,100200) spc=(600,0,100200) cnt=32 convRadius=632.456 gs0222
greenMpole( 0,  3): value= 0.030381186 term= 0.030381186 eps=1.000000
greenMpole( 1,  5): value= 0.030379909 term=-0.000001277 eps=0.000042
greenPwc: CollocationGreenPwc(val= 0.030379909) val= 0.049667462
greenPwc: d=0 r=4800 sp1=(600,1200,100000) sp2=(600,1200,100000) E-E
greenMpole: spo=(600,1200,100000) spc=(600,1200,100000) cnt=33 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.001025403 term= 0.001025403 eps=1.000000
greenMpole( 1,  5): value= 0.001024122 term=-0.000001281 eps=0.001251
greenMpole( 2,  7): value= 0.001024549 term= 0.000000427 eps=0.000417
greenPwc: CollocationGreenPwc(val= 0.001024549)
greenPwc: d=632.456 r=4800 sp1=(600,1200,100000) sp2=(0,1200,100200) E-F
greenMpole: spo=(600,1200,100000) spc=(0,1200,100200) cnt=34 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 0.000589354 term= 0.000589354 eps=1.000000
greenMpole( 1,  5): value= 0.000588075 term=-0.000001279 eps=0.002175
greenMpole( 2,  7): value= 0.000588501 term= 0.000000426 eps=0.000725
greenPwc: CollocationGreenPwc(val= 0.000588501)
greenMpole: spo=(0,1200,100200) spc=(600,1200,100000) cnt=35 convRadius=1341.64 gs0222
greenMpole( 0,  3): value= 0.000647049 term= 0.000647049 eps=1.000000
greenMpole( 1,  5): value= 0.000645770 term=-0.000001279 eps=0.001981
greenMpole( 2,  7): value= 0.000646197 term= 0.000000426 eps=0.000660
greenPwc: CollocationGreenPwc(val= 0.000646197) val= 0.000617349
greenPwc: d=0 r=4800 sp1=(0,1200,100200) sp2=(0,1200,100200) F-F
greenMpole: spo=(0,1200,100200) spc=(0,1200,100200) cnt=36 convRadius=1216.55 gs0222
greenMpole( 0,  3): value= 1.352229489 term= 1.352229489 eps=1.000000
greenMpole( 1,  5): value= 1.352228212 term=-0.000001277 eps=0.000001
greenPwc: CollocationGreenPwc(val= 1.352228212)

overall resource utilization:
        memory allocation  : 0.18 Mbyte
	user time          :         0.0
	system time        :         0.0
-------------------------------------------------------------------------------

/* Date: 9-Apr-02 15:08:11 GMT */
network single2 ()
{
    cap 271.6458e-18 (SUBSTR, GND);
}

-------------------------------------------------------------------------------
Note: By use_mean_green_values=1

/* Date: 9-Apr-02 15:28:12 GMT */
network single2 ()
{
    cap 271.7024e-18 (SUBSTR, GND);
}
-------------------------------------------------------------------------------
.C0
.SK
.HU "APPENDIX C -- Other Results: use_old_images=0"
.C8
-------------------------------------------------------------------------------
greenInit: i=1 m='SiO2' e=3.9 b=0
greenInit: i=2 m='air' e=0.01 b=250
-------------------------------------------------------------------------------
/* Date: 10-Apr-02 9:48:03 GMT */
network single2 ()
{
    cap 273.3789e-18 (SUBSTR, GND); /*mean: 273.4352e-18 */
}
-------------------------------------------------------------------------------
greenInit: i=1 m='SiO2' e=3.9 b=0
greenInit: i=2 m='air' e=0.1 b=250
-------------------------------------------------------------------------------
/* Date: 10-Apr-02 9:51:32 GMT */
network single2 ()
{
    cap 290.3342e-18 (SUBSTR, GND); /*mean: 290.391e-18 */
}
-------------------------------------------------------------------------------
greenInit: i=1 m='SiO2' e=3.9 b=0
greenInit: i=2 m='air' e=1 b=250
-------------------------------------------------------------------------------
/* Date: 10-Apr-02 9:54:14 GMT */
network single2 ()
{
    cap 437.1283e-18 (SUBSTR, GND); /*mean: 437.1555e-18 */
}
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
greenInit: i=1 m='SiO2' e=3.9 b=0
greenInit: i=2 m='air' e=0.001 b=250
-------------------------------------------------------------------------------
vdimensions:
  metal1v  : cmf     : cmf  : 250 2   # double thickness of interconnect

/* Date: 11-Apr-02 10:45:42 GMT */
network single2 ()
{
    cap 271.6751e-18 (SUBSTR, GND);
}
-------------------------------------------------------------------------------
.C0
.SK
.HU "APPENDIX D -- Results: SETUP-B"
.C8
extracting single2
greenInit: use_lowest_medium=1 use_old_images=0
greenInit: use_mean_green_values=0 merge_images=1
greenInit: use_multipoles=1 test_multipoles=0 min_divergence_term=10
greenInit: be_mode='collocation' FeModeGalerkin=0 FeModePwl=0
greenInit: reading dielectric specification
greenInit: i=1 m='SiO2' e=3.9 b=0
greenInit: i=2 m='air' e=0.001 b=250
greenInit: greenType=0 nLayers=2 maxGreenTerms=100 collocationEps=0.001
strip 0 300 (add)
Schur dimension 10, maxorder 9
greenPwc: d=0 r=4800 sp1=(1200,1200,100200) sp2=(1200,1200,100200) A-A
greenMpole: spo=(1200,1200,100200) spc=(1200,1200,100200) cnt=1 convRadius=1216.55 gs0222
greenMpole( 1,  5): value= 1.352228212 term=-0.000001277 eps=0.000001
greenPwc: d=400 r=4800 sp1=(1200,1200,100200) sp2=(1200,1200,99800) A-B
greenMpole: spo=(1200,1200,100200) spc=(1200,1200,99800) cnt=2 convRadius=1216.55 gs0212
greenMpole( 2,  6): value= 0.000793590 term= 0.000000428 eps=0.000539
greenMpole: spo=(1200,1200,99800) spc=(1200,1200,100200) cnt=3 convRadius=1216.55 gs0221
greenMpole( 2,  6): value= 0.000793590 term= 0.000000428 eps=0.000539
greenPwc: d=632.456 r=4800 sp1=(1200,1200,100200) sp2=(600,1200,100400) A-C
greenMpole: spo=(1200,1200,100200) spc=(600,1200,100400) cnt=4 convRadius=1341.64 gs0222
greenMpole( 1,  5): value= 0.312724870 term=-0.000001275 eps=0.000004
greenMpole: spo=(600,1200,100400) spc=(1200,1200,100200) cnt=5 convRadius=1216.55 gs0222
greenMpole( 1,  5): value= 0.198914798 term=-0.000001275 eps=0.000006
greenPwc: d=1341.64 r=3600 sp1=(1200,1200,100200) sp2=(600,2400,100200) A-D
greenMpole: spo=(1200,1200,100200) spc=(600,2400,100200) cnt=6 convRadius=632.456 gs0222
greenMpole( 1,  5): value= 0.030379909 term=-0.000001277 eps=0.000042
greenMpole: spo=(600,2400,100200) spc=(1200,1200,100200) cnt=7 convRadius=1216.55 gs0222
greenMpole( 1,  5): value= 0.068955015 term=-0.000001277 eps=0.000019
greenPwc: d=1341.64 r=3600 sp1=(1200,1200,100200) sp2=(600,0,100200) A-E
greenMpole: spo=(1200,1200,100200) spc=(600,0,100200) cnt=8 convRadius=632.456 gs0222
greenMpole( 1,  5): value= 0.030379909 term=-0.000001277 eps=0.000042
greenMpole: spo=(600,0,100200) spc=(1200,1200,100200) cnt=9 convRadius=1216.55 gs0222
greenMpole( 1,  5): value= 0.068955015 term=-0.000001277 eps=0.000019
greenPwc: d=1400 r=3600 sp1=(1200,1200,100200) sp2=(600,2400,99800) A-F
greenMpole: spo=(1200,1200,100200) spc=(600,2400,99800) cnt=10 convRadius=632.456 gs0212
greenMpole( 3,  8): value= 0.000356597 term=-0.000000214 eps=0.000600
greenMpole: spo=(600,2400,99800) spc=(1200,1200,100200) cnt=11 convRadius=1216.55 gs0221
greenMpole( 3,  8): value= 0.000404980 term=-0.000000214 eps=0.000528
greenPwc: d=1400 r=3600 sp1=(1200,1200,100200) sp2=(600,0,99800) A-G
greenMpole: spo=(1200,1200,100200) spc=(600,0,99800) cnt=12 convRadius=632.456 gs0212
greenMpole( 3,  8): value= 0.000356597 term=-0.000000214 eps=0.000600
greenMpole: spo=(600,0,99800) spc=(1200,1200,100200) cnt=13 convRadius=1216.55 gs0221
greenMpole( 3,  8): value= 0.000404980 term=-0.000000214 eps=0.000528
greenPwc: d=848.528 r=4800 sp1=(1200,1200,100200) sp2=(600,1200,99600) A-H
greenMpole: spo=(1200,1200,100200) spc=(600,1200,99600) cnt=14 convRadius=1341.64 gs0212
greenMpole( 2,  6): value= 0.000486103 term= 0.000000428 eps=0.000881
greenMpole: spo=(600,1200,99600) spc=(1200,1200,100200) cnt=15 convRadius=1216.55 gs0221
greenMpole( 2,  6): value= 0.000486757 term= 0.000000428 eps=0.000880
greenPwc: d=1200 r=4800 sp1=(1200,1200,100200) sp2=(0,1200,100200) A-I
greenMpole: spo=(1200,1200,100200) spc=(0,1200,100200) cnt=16 convRadius=1216.55 gs0222
greenMpole( 1,  5): value= 0.030066782 term=-0.000001277 eps=0.000042
greenMpole: spo=(0,1200,100200) spc=(1200,1200,100200) cnt=17 convRadius=1216.55 gs0222
greenMpole( 1,  5): value= 0.030066782 term=-0.000001277 eps=0.000042
greenPwc: d=1264.91 r=4800 sp1=(1200,1200,100200) sp2=(0,1200,99800) A-J
greenMpole: spo=(1200,1200,100200) spc=(0,1200,99800) cnt=18 convRadius=1216.55 gs0212
greenMpole( 3,  8): value= 0.000356307 term=-0.000000214 eps=0.000600
greenMpole: spo=(0,1200,99800) spc=(1200,1200,100200) cnt=19 convRadius=1216.55 gs0221
greenMpole( 3,  8): value= 0.000356307 term=-0.000000214 eps=0.000600
greenPwc: d=0 r=4800 sp1=(1200,1200,99800) sp2=(1200,1200,99800) B-B
greenMpole: spo=(1200,1200,99800) spc=(1200,1200,99800) cnt=20 convRadius=1216.55 gs0211
greenMpole( 2, 10): value= 0.001139903 term=-0.000000425 eps=0.000373
greenPwc: d=848.528 r=4800 sp1=(1200,1200,99800) sp2=(600,1200,100400) B-C
greenMpole: spo=(1200,1200,99800) spc=(600,1200,100400) cnt=21 convRadius=1341.64 gs0221
greenMpole( 2,  6): value= 0.000486107 term= 0.000000429 eps=0.000883
greenMpole: spo=(600,1200,100400) spc=(1200,1200,99800) cnt=22 convRadius=1216.55 gs0212
greenMpole( 2,  6): value= 0.000486761 term= 0.000000429 eps=0.000881
greenPwc: d=1400 r=3600 sp1=(1200,1200,99800) sp2=(600,2400,100200) B-D
greenMpole: spo=(1200,1200,99800) spc=(600,2400,100200) cnt=23 convRadius=632.456 gs0221
greenMpole( 3,  8): value= 0.000356597 term=-0.000000214 eps=0.000600
greenMpole: spo=(600,2400,100200) spc=(1200,1200,99800) cnt=24 convRadius=1216.55 gs0212
greenMpole( 3,  8): value= 0.000404980 term=-0.000000214 eps=0.000528
greenPwc: d=1400 r=3600 sp1=(1200,1200,99800) sp2=(600,0,100200) B-E
greenMpole: spo=(1200,1200,99800) spc=(600,0,100200) cnt=25 convRadius=632.456 gs0221
greenMpole( 3,  8): value= 0.000356597 term=-0.000000214 eps=0.000600
greenMpole: spo=(600,0,100200) spc=(1200,1200,99800) cnt=26 convRadius=1216.55 gs0212
greenMpole( 3,  8): value= 0.000404980 term=-0.000000214 eps=0.000528
greenPwc: d=1341.64 r=3600 sp1=(1200,1200,99800) sp2=(600,2400,99800) B-F
greenMpole: spo=(1200,1200,99800) spc=(600,2400,99800) cnt=27 convRadius=632.456 gs0211
greenMpole( 3, 14): value= 0.000364407 term= 0.000000106 eps=0.000291
greenMpole: spo=(600,2400,99800) spc=(1200,1200,99800) cnt=28 convRadius=1216.55 gs0211
greenMpole( 3, 14): value= 0.000422668 term= 0.000000106 eps=0.000251
greenPwc: d=1341.64 r=3600 sp1=(1200,1200,99800) sp2=(600,0,99800) B-G
greenMpole: spo=(1200,1200,99800) spc=(600,0,99800) cnt=29 convRadius=632.456 gs0211
greenMpole( 3, 14): value= 0.000364407 term= 0.000000106 eps=0.000291
greenMpole: spo=(600,0,99800) spc=(1200,1200,99800) cnt=30 convRadius=1216.55 gs0211
greenMpole( 3, 14): value= 0.000422668 term= 0.000000106 eps=0.000251
greenPwc: d=632.456 r=4800 sp1=(1200,1200,99800) sp2=(600,1200,99600) B-H
greenMpole: spo=(1200,1200,99800) spc=(600,1200,99600) cnt=31 convRadius=1341.64 gs0211
greenMpole( 2, 10): value= 0.000565959 term=-0.000000423 eps=0.000748
greenMpole: spo=(600,1200,99600) spc=(1200,1200,99800) cnt=32 convRadius=1216.55 gs0211
greenMpole( 2, 10): value= 0.000537431 term=-0.000000423 eps=0.000788
greenPwc: d=1264.91 r=4800 sp1=(1200,1200,99800) sp2=(0,1200,100200) B-I
greenMpole: spo=(1200,1200,99800) spc=(0,1200,100200) cnt=33 convRadius=1216.55 gs0221
greenMpole( 3,  8): value= 0.000356307 term=-0.000000214 eps=0.000600
greenMpole: spo=(0,1200,100200) spc=(1200,1200,99800) cnt=34 convRadius=1216.55 gs0212
greenMpole( 3,  8): value= 0.000356307 term=-0.000000214 eps=0.000600
greenPwc: d=1200 r=4800 sp1=(1200,1200,99800) sp2=(0,1200,99800) B-J
greenMpole: spo=(1200,1200,99800) spc=(0,1200,99800) cnt=35 convRadius=1216.55 gs0211
greenMpole( 3, 14): value= 0.000364036 term= 0.000000106 eps=0.000292
greenMpole: spo=(0,1200,99800) spc=(1200,1200,99800) cnt=36 convRadius=1216.55 gs0211
greenMpole( 3, 14): value= 0.000364036 term= 0.000000106 eps=0.000292
greenPwc: d=0 r=4800 sp1=(600,1200,100400) sp2=(600,1200,100400) C-C
greenMpole: spo=(600,1200,100400) spc=(600,1200,100400) cnt=37 convRadius=1341.64 gs0222
greenMpole( 1,  5): value= 1.064884295 term=-0.000001273 eps=0.000001
greenPwc: d=1216.55 r=3600 sp1=(600,1200,100400) sp2=(600,2400,100200) C-D
greenMpole: spo=(600,1200,100400) spc=(600,2400,100200) cnt=38 convRadius=632.456 gs0222
greenMpole( 1,  5): value= 0.069782107 term=-0.000001275 eps=0.000018
greenMpole: spo=(600,2400,100200) spc=(600,1200,100400) cnt=39 convRadius=1341.64 gs0222
greenMpole( 1,  5): value= 0.271084265 term=-0.000001275 eps=0.000005
greenPwc: d=1216.55 r=3600 sp1=(600,1200,100400) sp2=(600,0,100200) C-E
greenMpole: spo=(600,1200,100400) spc=(600,0,100200) cnt=40 convRadius=632.456 gs0222
greenMpole( 1,  5): value= 0.069782107 term=-0.000001275 eps=0.000018
greenMpole: spo=(600,0,100200) spc=(600,1200,100400) cnt=41 convRadius=1341.64 gs0222
greenMpole( 1,  5): value= 0.271084265 term=-0.000001275 eps=0.000005
greenPwc: d=1341.64 r=3600 sp1=(600,1200,100400) sp2=(600,2400,99800) C-F
greenMpole: spo=(600,1200,100400) spc=(600,2400,99800) cnt=42 convRadius=632.456 gs0212
greenMpole( 3,  8): value= 0.000365196 term=-0.000000214 eps=0.000587
greenMpole: spo=(600,2400,99800) spc=(600,1200,100400) cnt=43 convRadius=1341.64 gs0221
greenMpole( 3,  8): value= 0.000416695 term=-0.000000214 eps=0.000514
greenPwc: d=1341.64 r=3600 sp1=(600,1200,100400) sp2=(600,0,99800) C-G
greenMpole: spo=(600,1200,100400) spc=(600,0,99800) cnt=44 convRadius=632.456 gs0212
greenMpole( 3,  8): value= 0.000365196 term=-0.000000214 eps=0.000587
greenMpole: spo=(600,0,99800) spc=(600,1200,100400) cnt=45 convRadius=1341.64 gs0221
greenMpole( 3,  8): value= 0.000416695 term=-0.000000214 eps=0.000514
greenPwc: d=800 r=4800 sp1=(600,1200,100400) sp2=(600,1200,99600) C-H
greenMpole: spo=(600,1200,100400) spc=(600,1200,99600) cnt=46 convRadius=1341.64 gs0212
greenMpole( 2,  6): value= 0.000478832 term= 0.000000429 eps=0.000897
greenMpole: spo=(600,1200,99600) spc=(600,1200,100400) cnt=47 convRadius=1341.64 gs0221
greenMpole( 2,  6): value= 0.000478832 term= 0.000000429 eps=0.000897
greenPwc: d=632.456 r=4800 sp1=(600,1200,100400) sp2=(0,1200,100200) C-I
greenMpole: spo=(600,1200,100400) spc=(0,1200,100200) cnt=48 convRadius=1216.55 gs0222
greenMpole( 1,  5): value= 0.198914798 term=-0.000001275 eps=0.000006
greenMpole: spo=(0,1200,100200) spc=(600,1200,100400) cnt=49 convRadius=1341.64 gs0222
greenMpole( 1,  5): value= 0.312724870 term=-0.000001275 eps=0.000004
greenPwc: d=848.528 r=4800 sp1=(600,1200,100400) sp2=(0,1200,99800) C-J
greenMpole: spo=(600,1200,100400) spc=(0,1200,99800) cnt=50 convRadius=1216.55 gs0212
greenMpole( 2,  6): value= 0.000486761 term= 0.000000429 eps=0.000881
greenMpole: spo=(0,1200,99800) spc=(600,1200,100400) cnt=51 convRadius=1341.64 gs0221
greenMpole( 2,  6): value= 0.000486107 term= 0.000000429 eps=0.000883
greenPwc: d=0 r=2400 sp1=(600,2400,100200) sp2=(600,2400,100200) D-D
greenMpole: spo=(600,2400,100200) spc=(600,2400,100200) cnt=52 convRadius=632.456 gs0222
greenMpole( 1,  5): value= 2.596183242 term=-0.000001277 eps=0.000000
greenPwc: d=2400 r=2400 sp1=(600,2400,100200) sp2=(600,0,100200) D-E
greenMpole: spo=(600,2400,100200) spc=(600,0,100200) cnt=53 convRadius=632.456 gs0222
greenMpole( 1,  5): value= 0.005642015 term=-0.000001277 eps=0.000226
greenMpole: spo=(600,0,100200) spc=(600,2400,100200) cnt=54 convRadius=632.456 gs0222
greenMpole( 1,  5): value= 0.005642015 term=-0.000001277 eps=0.000226
greenPwc: d=400 r=2400 sp1=(600,2400,100200) sp2=(600,2400,99800) D-F
greenMpole: spo=(600,2400,100200) spc=(600,2400,99800) cnt=55 convRadius=632.456 gs0212
greenMpole( 2,  6): value= 0.001059316 term= 0.000000428 eps=0.000404
greenMpole: spo=(600,2400,99800) spc=(600,2400,100200) cnt=56 convRadius=632.456 gs0221
greenMpole( 2,  6): value= 0.001059316 term= 0.000000428 eps=0.000404
greenPwc: d=2433.11 r=2400 sp1=(600,2400,100200) sp2=(600,0,99800) D-G
greenMpole: spo=(600,2400,100200) spc=(600,0,99800) cnt=57 convRadius=632.456 gs0212
greenMpole( 4, 10): value= 0.000204853 term= 0.000000128 eps=0.000626
greenMpole: spo=(600,0,99800) spc=(600,2400,100200) cnt=58 convRadius=632.456 gs0221
greenMpole( 4, 10): value= 0.000204853 term= 0.000000128 eps=0.000626
greenPwc: d=1341.64 r=3600 sp1=(600,2400,100200) sp2=(600,1200,99600) D-H
greenMpole: spo=(600,2400,100200) spc=(600,1200,99600) cnt=59 convRadius=1341.64 gs0212
greenMpole( 3,  8): value= 0.000416691 term=-0.000000214 eps=0.000514
greenMpole: spo=(600,1200,99600) spc=(600,2400,100200) cnt=60 convRadius=632.456 gs0221
greenMpole( 3,  8): value= 0.000365192 term=-0.000000214 eps=0.000586
greenPwc: d=1341.64 r=3600 sp1=(600,2400,100200) sp2=(0,1200,100200) D-I
greenMpole: spo=(600,2400,100200) spc=(0,1200,100200) cnt=61 convRadius=1216.55 gs0222
greenMpole( 1,  5): value= 0.068955015 term=-0.000001277 eps=0.000019
greenMpole: spo=(0,1200,100200) spc=(600,2400,100200) cnt=62 convRadius=632.456 gs0222
greenMpole( 1,  5): value= 0.030379909 term=-0.000001277 eps=0.000042
greenPwc: d=1400 r=3600 sp1=(600,2400,100200) sp2=(0,1200,99800) D-J
greenMpole: spo=(600,2400,100200) spc=(0,1200,99800) cnt=63 convRadius=1216.55 gs0212
greenMpole( 3,  8): value= 0.000404980 term=-0.000000214 eps=0.000528
greenMpole: spo=(0,1200,99800) spc=(600,2400,100200) cnt=64 convRadius=632.456 gs0221
greenMpole( 3,  8): value= 0.000356597 term=-0.000000214 eps=0.000600
greenPwc: d=0 r=2400 sp1=(600,0,100200) sp2=(600,0,100200) E-E
greenMpole: spo=(600,0,100200) spc=(600,0,100200) cnt=65 convRadius=632.456 gs0222
greenMpole( 1,  5): value= 2.596183242 term=-0.000001277 eps=0.000000
greenPwc: d=2433.11 r=2400 sp1=(600,0,100200) sp2=(600,2400,99800) E-F
greenMpole: spo=(600,0,100200) spc=(600,2400,99800) cnt=66 convRadius=632.456 gs0212
greenMpole( 4, 10): value= 0.000204853 term= 0.000000128 eps=0.000626
greenMpole: spo=(600,2400,99800) spc=(600,0,100200) cnt=67 convRadius=632.456 gs0221
greenMpole( 4, 10): value= 0.000204853 term= 0.000000128 eps=0.000626
greenPwc: d=400 r=2400 sp1=(600,0,100200) sp2=(600,0,99800) E-G
greenMpole: spo=(600,0,100200) spc=(600,0,99800) cnt=68 convRadius=632.456 gs0212
greenMpole( 2,  6): value= 0.001059316 term= 0.000000428 eps=0.000404
greenMpole: spo=(600,0,99800) spc=(600,0,100200) cnt=69 convRadius=632.456 gs0221
greenMpole( 2,  6): value= 0.001059316 term= 0.000000428 eps=0.000404
greenPwc: d=1341.64 r=3600 sp1=(600,0,100200) sp2=(600,1200,99600) E-H
greenMpole: spo=(600,0,100200) spc=(600,1200,99600) cnt=70 convRadius=1341.64 gs0212
greenMpole( 3,  8): value= 0.000416691 term=-0.000000214 eps=0.000514
greenMpole: spo=(600,1200,99600) spc=(600,0,100200) cnt=71 convRadius=632.456 gs0221
greenMpole( 3,  8): value= 0.000365192 term=-0.000000214 eps=0.000586
greenPwc: d=1341.64 r=3600 sp1=(600,0,100200) sp2=(0,1200,100200) E-I
greenMpole: spo=(600,0,100200) spc=(0,1200,100200) cnt=72 convRadius=1216.55 gs0222
greenMpole( 1,  5): value= 0.068955015 term=-0.000001277 eps=0.000019
greenMpole: spo=(0,1200,100200) spc=(600,0,100200) cnt=73 convRadius=632.456 gs0222
greenMpole( 1,  5): value= 0.030379909 term=-0.000001277 eps=0.000042
greenPwc: d=1400 r=3600 sp1=(600,0,100200) sp2=(0,1200,99800) E-J
greenMpole: spo=(600,0,100200) spc=(0,1200,99800) cnt=74 convRadius=1216.55 gs0212
greenMpole( 3,  8): value= 0.000404980 term=-0.000000214 eps=0.000528
greenMpole: spo=(0,1200,99800) spc=(600,0,100200) cnt=75 convRadius=632.456 gs0221
greenMpole( 3,  8): value= 0.000356597 term=-0.000000214 eps=0.000600
greenPwc: d=0 r=2400 sp1=(600,2400,99800) sp2=(600,2400,99800) F-F
greenMpole: spo=(600,2400,99800) spc=(600,2400,99800) cnt=76 convRadius=632.456 gs0211
greenMpole( 2, 10): value= 0.001724524 term=-0.000000425 eps=0.000246
greenPwc: d=2400 r=2400 sp1=(600,2400,99800) sp2=(600,0,99800) F-G
greenMpole: spo=(600,2400,99800) spc=(600,0,99800) cnt=77 convRadius=632.456 gs0211
greenMpole( 3, 14): value= 0.000206230 term= 0.000000106 eps=0.000515
greenMpole: spo=(600,0,99800) spc=(600,2400,99800) cnt=78 convRadius=632.456 gs0211
greenMpole( 3, 14): value= 0.000206230 term= 0.000000106 eps=0.000515
greenPwc: d=1216.55 r=3600 sp1=(600,2400,99800) sp2=(600,1200,99600) F-H
greenMpole: spo=(600,2400,99800) spc=(600,1200,99600) cnt=79 convRadius=1341.64 gs0211
greenMpole( 2, 10): value= 0.000486102 term=-0.000000423 eps=0.000871
greenMpole: spo=(600,1200,99600) spc=(600,2400,99800) cnt=80 convRadius=632.456 gs0211
greenMpole( 3, 14): value= 0.000383107 term= 0.000000106 eps=0.000277
greenPwc: d=1400 r=3600 sp1=(600,2400,99800) sp2=(0,1200,100200) F-I
greenMpole: spo=(600,2400,99800) spc=(0,1200,100200) cnt=81 convRadius=1216.55 gs0221
greenMpole( 3,  8): value= 0.000404980 term=-0.000000214 eps=0.000528
greenMpole: spo=(0,1200,100200) spc=(600,2400,99800) cnt=82 convRadius=632.456 gs0212
greenMpole( 3,  8): value= 0.000356597 term=-0.000000214 eps=0.000600
greenPwc: d=1341.64 r=3600 sp1=(600,2400,99800) sp2=(0,1200,99800) F-J
greenMpole: spo=(600,2400,99800) spc=(0,1200,99800) cnt=83 convRadius=1216.55 gs0211
greenMpole( 3, 14): value= 0.000422668 term= 0.000000106 eps=0.000251
greenMpole: spo=(0,1200,99800) spc=(600,2400,99800) cnt=84 convRadius=632.456 gs0211
greenMpole( 3, 14): value= 0.000364407 term= 0.000000106 eps=0.000291
greenPwc: d=0 r=2400 sp1=(600,0,99800) sp2=(600,0,99800) G-G
greenMpole: spo=(600,0,99800) spc=(600,0,99800) cnt=85 convRadius=632.456 gs0211
greenMpole( 2, 10): value= 0.001724524 term=-0.000000425 eps=0.000246
greenPwc: d=1216.55 r=3600 sp1=(600,0,99800) sp2=(600,1200,99600) G-H
greenMpole: spo=(600,0,99800) spc=(600,1200,99600) cnt=86 convRadius=1341.64 gs0211
greenMpole( 2, 10): value= 0.000486102 term=-0.000000423 eps=0.000871
greenMpole: spo=(600,1200,99600) spc=(600,0,99800) cnt=87 convRadius=632.456 gs0211
greenMpole( 3, 14): value= 0.000383107 term= 0.000000106 eps=0.000277
greenPwc: d=1400 r=3600 sp1=(600,0,99800) sp2=(0,1200,100200) G-I
greenMpole: spo=(600,0,99800) spc=(0,1200,100200) cnt=88 convRadius=1216.55 gs0221
greenMpole( 3,  8): value= 0.000404980 term=-0.000000214 eps=0.000528
greenMpole: spo=(0,1200,100200) spc=(600,0,99800) cnt=89 convRadius=632.456 gs0212
greenMpole( 3,  8): value= 0.000356597 term=-0.000000214 eps=0.000600
greenPwc: d=1341.64 r=3600 sp1=(600,0,99800) sp2=(0,1200,99800) G-J
greenMpole: spo=(600,0,99800) spc=(0,1200,99800) cnt=90 convRadius=1216.55 gs0211
greenMpole( 3, 14): value= 0.000422668 term= 0.000000106 eps=0.000251
greenMpole: spo=(0,1200,99800) spc=(600,0,99800) cnt=91 convRadius=632.456 gs0211
greenMpole( 3, 14): value= 0.000364407 term= 0.000000106 eps=0.000291
greenPwc: d=0 r=4800 sp1=(600,1200,99600) sp2=(600,1200,99600) H-H
greenMpole: spo=(600,1200,99600) spc=(600,1200,99600) cnt=92 convRadius=1341.64 gs0211
greenMpole( 2, 10): value= 0.000751552 term=-0.000000422 eps=0.000562
greenPwc: d=848.528 r=4800 sp1=(600,1200,99600) sp2=(0,1200,100200) H-I
greenMpole: spo=(600,1200,99600) spc=(0,1200,100200) cnt=93 convRadius=1216.55 gs0221
greenMpole( 2,  6): value= 0.000486757 term= 0.000000428 eps=0.000880
greenMpole: spo=(0,1200,100200) spc=(600,1200,99600) cnt=94 convRadius=1341.64 gs0212
greenMpole( 2,  6): value= 0.000486103 term= 0.000000428 eps=0.000881
greenPwc: d=632.456 r=4800 sp1=(600,1200,99600) sp2=(0,1200,99800) H-J
greenMpole: spo=(600,1200,99600) spc=(0,1200,99800) cnt=95 convRadius=1216.55 gs0211
greenMpole( 2, 10): value= 0.000537431 term=-0.000000423 eps=0.000788
greenMpole: spo=(0,1200,99800) spc=(600,1200,99600) cnt=96 convRadius=1341.64 gs0211
greenMpole( 2, 10): value= 0.000565959 term=-0.000000423 eps=0.000748
greenPwc: d=0 r=4800 sp1=(0,1200,100200) sp2=(0,1200,100200) I-I
greenMpole: spo=(0,1200,100200) spc=(0,1200,100200) cnt=97 convRadius=1216.55 gs0222
greenMpole( 1,  5): value= 1.352228212 term=-0.000001277 eps=0.000001
greenPwc: d=400 r=4800 sp1=(0,1200,100200) sp2=(0,1200,99800) I-J
greenMpole: spo=(0,1200,100200) spc=(0,1200,99800) cnt=98 convRadius=1216.55 gs0212
greenMpole( 2,  6): value= 0.000793590 term= 0.000000428 eps=0.000539
greenMpole: spo=(0,1200,99800) spc=(0,1200,100200) cnt=99 convRadius=1216.55 gs0221
greenMpole( 2,  6): value= 0.000793590 term= 0.000000428 eps=0.000539
greenPwc: d=0 r=4800 sp1=(0,1200,99800) sp2=(0,1200,99800) J-J
greenMpole: spo=(0,1200,99800) spc=(0,1200,99800) cnt=100 convRadius=1216.55 gs0211
greenMpole( 2, 10): value= 0.001139903 term=-0.000000425 eps=0.000373

-------------------------------------------------------------------------------
/* Date: 11-Apr-02 11:54:53 GMT */
network single2 ()
{
    cap 479.8535e-18 (SUBSTR, GND);
}
-------------------------------------------------------------------------------
.C0
.SK
.HU "APPENDIX E -- Results diff: SETUP-B new/old"
.C8
-------------------------------------------------------------------------------
greenInit: use_lowest_medium=1 use_old_images=0

overall resource utilization:
        memory allocation  : 0.188 Mbyte
	user time          :         0.1
	system time        :         0.2
	real time          :         0.5  67%

network single2 ()
{
    cap 479.8535e-18 (SUBSTR, GND);
}
-------------------------------------------------------------------------------
greenInit: use_lowest_medium=1 use_old_images=1

space3d: Warning: maximum error not reached for 23.0% of the Greens functions.

overall resource utilization:
        memory allocation  : 0.2 Mbyte
	user time          :         0.2
	system time        :         0.5
	real time          :         0.9  76%

network single2 ()
{
    cap 479.865e-18 (SUBSTR, GND);
}
-------------------------------------------------------------------------------

.C0
.HU "APPENDIX F -- Results: SETUP-B for different vdimensions"
.C1
    vdim=252,2 cap=364.7354e-21
    vdim=251,2 cap=469.9384e-21
    vdim=250,2 cap=271.6751e-18
    vdim=249,2 cap=479.8535e-18
    vdim=248,2 cap=581.6593e-18
    vdim=247,2 cap=659.8488e-18
    vdim=246,2 cap=716.7649e-18
    vdim=245,2 cap=757.4764e-18
    vdim=244,2 cap=786.9681e-18
    vdim=243,2 cap=809.0504e-18
    vdim=242,2 cap=826.1002e-18
    vdim=241,2 cap=839.6160e-18
    vdim=240,2 cap=850.5725e-18
.C0
