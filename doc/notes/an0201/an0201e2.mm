.T= "Cap3D Extraction Example2"
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
EXAMPLE2 (single2, nLayers=3)
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
Report ET-CAS 02-01-2
.ce
April 12, 2002
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2002-2004 by the author.

Last revision: December 4, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "EXAMPLE2 CELL: single2"
.H 2 "single2.ldm"
.C1
:: lambda = 0.01 micron
ms single2
box cmf 0 300 0 600
me
.C0
.F+
.PSPIC an0201/e2_f1.ps 14c
.F-
.F+
.PSPIC an0201/e2_f2.ps 8c
.F-

.H 1 "EXAMPLE2 SETUP"
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
    SiX  2.0    100
    air  0.001  250 
.C0
.H 2 "space.green.p"
.C1
lat_cap_window   3000.0 micron

#min_divergence_term   10
#cap3d.use_old_images           on
#cap3d.use_lowest_medium        off
#cap3d.use_mean_green_values    on

#cap3d.green_eps 1e-8
cap3d.max_green_terms 100
cap3d.max_be_area               inf
cap3d.be_window                 inf

debug.print_green_init
debug.print_green_terms
#debug.print_green_gterms
.C0
.H 2 "execution statement"
.C1
space3d -vC3 -E space.green.t -P space.green.p single2
.C0

.H 1 "APPENDICES"
.nr Hu 2 \" Level of this .HU heading
.HU "APPENDIX A -- Results:"
.C8
-------------------------------------------------------------------------------
Note: use_old_images=1 is equal to use_old_images=0.
-------------------------------------------------------------------------------

extracting single2
greenInit: use_lowest_medium=1 use_old_images=0
greenInit: use_mean_green_values=0 merge_images=1
greenInit: use_multipoles=1 test_multipoles=0 min_divergence_term=3
greenInit: be_mode='collocation' FeModeGalerkin=0 FeModePwl=0
greenInit: reading dielectric specification
greenInit: i=1 m='SiO2' e=3.9 b=0
greenInit: i=2 m='SiX' e=2 b=100
greenInit: i=3 m='air' e=0.001 b=250
greenInit: greenType=0 nLayers=3 maxGreenTerms=100 collocationEps=0.001
strip 0 300 (add)
Schur dimension 6, maxorder 5
greenPwc: d=0 r=4800 sp1=(1200,1200,100200) sp2=(1200,1200,100200) A-A
greenMpole: spo=(1200,1200,100200) spc=(1200,1200,100200) cnt=1 convRadius=1216.55 gs0333
greenMpole( 0,  2): value= 2.901015970 term= 2.901015970 eps=1.000000
greenMpole( 1,  8): value= 1.345970500 term=-1.555045470 eps=1.155334
greenMpole( 2, 21): value= 1.356605164 term= 0.010634665 eps=0.007839
greenMpole( 3, 37): value= 1.350053933 term=-0.006551231 eps=0.004853
greenMpole( 4, 56): value= 1.355842209 term= 0.005788277 eps=0.004269
greenMpole( 5, 78): value= 1.349858346 term=-0.005983864 eps=0.004433
greenMpole: Greens function divergence after 6 terms (eps=0.00426914)
greenPwc: CollocationGreenPwc(val= 1.349858346)
greenPwc: d=632.456 r=4800 sp1=(1200,1200,100200) sp2=(600,1200,100400) A-B
greenMpole: spo=(1200,1200,100200) spc=(600,1200,100400) cnt=2 convRadius=1341.64 gs0333
greenMpole( 0,  2): value= 1.262079440 term= 1.262079440 eps=1.000000
greenMpole( 1,  8): value= 0.306192061 term=-0.955887379 eps=3.121856
greenMpole( 2, 21): value= 0.316805631 term= 0.010613569 eps=0.033502
greenMpole( 3, 37): value= 0.310260924 term=-0.006544707 eps=0.021094
greenMpole( 4, 56): value= 0.316045225 term= 0.005784301 eps=0.018302
greenMpole( 5, 78): value= 0.310064533 term=-0.005980692 eps=0.019289
greenMpole: Greens function divergence after 6 terms (eps=0.0183021)
greenPwc: CollocationGreenPwc(val= 0.310064533)
greenMpole: spo=(600,1200,100400) spc=(1200,1200,100200) cnt=3 convRadius=1216.55 gs0333
greenMpole( 0,  2): value= 1.149544593 term= 1.149544593 eps=1.000000
greenMpole( 1,  8): value= 0.192382546 term=-0.957162047 eps=4.975306
greenMpole( 2, 21): value= 0.202996188 term= 0.010613641 eps=0.052285
greenMpole( 3, 37): value= 0.196451471 term=-0.006544717 eps=0.033315
greenMpole( 4, 56): value= 0.202235776 term= 0.005784305 eps=0.028602
greenMpole( 5, 78): value= 0.196255082 term=-0.005980694 eps=0.030474
greenMpole: Greens function divergence after 6 terms (eps=0.0286018)
greenPwc: CollocationGreenPwc(val= 0.196255082) val= 0.253159807
greenPwc: d=1341.64 r=3600 sp1=(1200,1200,100200) sp2=(600,2400,100200) A-C
greenMpole: spo=(1200,1200,100200) spc=(600,2400,100200) cnt=4 convRadius=632.456 gs0333
greenMpole( 0,  2): value= 0.727667433 term= 0.727667433 eps=1.000000
greenMpole( 1,  8): value= 0.023708103 term=-0.703959330 eps=29.692774
greenMpole( 2, 21): value= 0.034342056 term= 0.010633953 eps=0.309648
greenMpole( 3, 37): value= 0.027790923 term=-0.006551133 eps=0.235729
greenMpole( 4, 56): value= 0.033579160 term= 0.005788237 eps=0.172376
greenMpole( 5, 78): value= 0.027595320 term=-0.005983840 eps=0.216843
greenMpole: Greens function divergence after 6 terms (eps=0.172376)
greenPwc: CollocationGreenPwc(val= 0.027595320)
greenMpole: spo=(600,2400,100200) spc=(1200,1200,100200) cnt=5 convRadius=1216.55 gs0333
greenMpole( 0,  2): value= 0.860565423 term= 0.860565423 eps=1.000000
greenMpole( 1,  8): value= 0.062329309 term=-0.798236114 eps=12.806754
greenMpole( 2, 21): value= 0.072963085 term= 0.010633776 eps=0.145742
greenMpole( 3, 37): value= 0.066411976 term=-0.006551109 eps=0.098643
greenMpole( 4, 56): value= 0.072200203 term= 0.005788228 eps=0.080169
greenMpole( 5, 78): value= 0.066216369 term=-0.005983834 eps=0.090368
greenMpole: Greens function divergence after 6 terms (eps=0.0801691)
greenPwc: CollocationGreenPwc(val= 0.066216369) val= 0.046905845
greenPwc: d=1341.64 r=3600 sp1=(1200,1200,100200) sp2=(600,0,100200) A-D
greenMpole: spo=(1200,1200,100200) spc=(600,0,100200) cnt=6 convRadius=632.456 gs0333
     ... (see A-C 1)
greenPwc: CollocationGreenPwc(val= 0.027595320)
greenMpole: spo=(600,0,100200) spc=(1200,1200,100200) cnt=7 convRadius=1216.55 gs0333
     ... (see A-C 2)
greenPwc: CollocationGreenPwc(val= 0.066216369) val= 0.046905845
greenPwc: d=632.456 r=4800 sp1=(1200,1200,100200) sp2=(600,1200,100000) A-E
greenMpole: spo=(1200,1200,100200) spc=(600,1200,100000) cnt=8 convRadius=1341.64 gs0323
greenMpole( 0,  2): value= 1.262069480 term= 1.262069480 eps=1.000000
greenMpole( 1, 11): value=-0.005807420 term=-1.267876900 eps=218.320176
greenMpole( 2, 29): value= 0.004899623 term= 0.010707042 eps=2.185279
greenMpole( 3, 53): value=-0.001679354 term=-0.006578977 eps=3.917563
greenMpole: Greens function divergence after 4 terms (eps=2.18528)
greenPwc: CollocationGreenPwc(val=-0.001679354)
greenMpole: spo=(600,1200,100000) spc=(1200,1200,100200) cnt=9 convRadius=1216.55 gs0332
greenMpole( 0,  2): value= 0.000574767 term= 0.000574767 eps=1.000000
greenMpole( 1, 11): value= 0.001146781 term= 0.000572013 eps=0.498799
greenMpole( 2, 29): value= 0.001144708 term=-0.000002073 eps=0.001811
greenMpole( 3, 53): value= 0.001145278 term= 0.000000570 eps=0.000498
greenPwc: CollocationGreenPwc(val= 0.001145278) val=-0.000267038
greenPwc: d=1200 r=4800 sp1=(1200,1200,100200) sp2=(0,1200,100200) A-F
greenMpole: spo=(1200,1200,100200) spc=(0,1200,100200) cnt=10 convRadius=1216.55 gs0333
greenMpole( 0,  2): value= 0.726788071 term= 0.726788071 eps=1.000000
greenMpole( 1,  8): value= 0.023394700 term=-0.703393371 eps=30.066356
greenMpole( 2, 21): value= 0.034028653 term= 0.010633953 eps=0.312500
greenMpole( 3, 37): value= 0.027477520 term=-0.006551133 eps=0.238418
greenMpole( 4, 56): value= 0.033265757 term= 0.005788237 eps=0.174000
greenMpole( 5, 78): value= 0.027281917 term=-0.005983840 eps=0.219334
greenMpole: Greens function divergence after 6 terms (eps=0.174)
greenPwc: CollocationGreenPwc(val= 0.027281917)
greenMpole: spo=(0,1200,100200) spc=(1200,1200,100200) cnt=11 convRadius=1216.55 gs0333
     ... (see A-F 1)
greenPwc: CollocationGreenPwc(val= 0.027281917) val= 0.027281917
greenPwc: d=0 r=4800 sp1=(600,1200,100400) sp2=(600,1200,100400) B-B
greenMpole: spo=(600,1200,100400) spc=(600,1200,100400) cnt=12 convRadius=1341.64 gs0333
greenMpole( 0,  2): value= 2.000069228 term= 2.000069228 eps=1.000000
greenMpole( 1,  8): value= 1.058360997 term=-0.941708232 eps=0.889780
greenMpole( 2, 21): value= 1.068953975 term= 0.010592979 eps=0.009910
greenMpole( 3, 37): value= 1.062415723 term=-0.006538253 eps=0.006154
greenMpole( 4, 56): value= 1.068196076 term= 0.005780353 eps=0.005411
greenMpole( 5, 78): value= 1.062218540 term=-0.005977536 eps=0.005627
greenMpole: Greens function divergence after 6 terms (eps=0.00541132)
greenPwc: CollocationGreenPwc(val= 1.062218540)
greenPwc: d=1216.55 r=3600 sp1=(600,1200,100400) sp2=(600,2400,100200) B-C
greenMpole: spo=(600,1200,100400) spc=(600,2400,100200) cnt=13 convRadius=632.456 gs0333
greenMpole( 0,  2): value= 0.783838718 term= 0.783838718 eps=1.000000
greenMpole( 1,  8): value= 0.063134947 term=-0.720703771 eps=11.415291
greenMpole( 2, 21): value= 0.073748235 term= 0.010613288 eps=0.143912
greenMpole( 3, 37): value= 0.067203567 term=-0.006544668 eps=0.097386
greenMpole( 4, 56): value= 0.072987852 term= 0.005784285 eps=0.079250
greenMpole( 5, 78): value= 0.067007170 term=-0.005980682 eps=0.089254
greenMpole: Greens function divergence after 6 terms (eps=0.07925)
greenPwc: CollocationGreenPwc(val= 0.067007170)
greenMpole: spo=(600,2400,100200) spc=(600,1200,100400) cnt=14 convRadius=1341.64 gs0333
greenMpole( 0,  2): value= 1.085537994 term= 1.085537994 eps=1.000000
greenMpole( 1,  8): value= 0.264486228 term=-0.821051766 eps=3.104327
greenMpole( 2, 21): value= 0.275099267 term= 0.010613039 eps=0.038579
greenMpole( 3, 37): value= 0.268554633 term=-0.006544634 eps=0.024370
greenMpole( 4, 56): value= 0.274338905 term= 0.005784272 eps=0.021084
greenMpole( 5, 78): value= 0.268358231 term=-0.005980674 eps=0.022286
greenMpole: Greens function divergence after 6 terms (eps=0.0210844)
greenPwc: CollocationGreenPwc(val= 0.268358231) val= 0.167682701
greenPwc: d=1216.55 r=3600 sp1=(600,1200,100400) sp2=(600,0,100200) B-D
greenMpole: spo=(600,1200,100400) spc=(600,0,100200) cnt=15 convRadius=632.456 gs0333
     ... (see B-C 1)
greenPwc: CollocationGreenPwc(val= 0.067007170)
greenMpole: spo=(600,0,100200) spc=(600,1200,100400) cnt=16 convRadius=1341.64 gs0333
     ... (see B-C 2)
greenPwc: CollocationGreenPwc(val= 0.268358231) val= 0.167682701
greenPwc: d=400 r=4800 sp1=(600,1200,100400) sp2=(600,1200,100000) B-E
greenMpole: spo=(600,1200,100400) spc=(600,1200,100000) cnt=17 convRadius=1341.64 gs0323
greenMpole( 0,  2): value= 1.321896105 term= 1.321896105 eps=1.000000
greenMpole( 1, 11): value=-0.005770104 term=-1.327666209 eps=230.094002
greenMpole( 2, 29): value= 0.004967902 term= 0.010738006 eps=2.161477
greenMpole( 3, 53): value=-0.001625878 term=-0.006593781 eps=4.055520
greenMpole: Greens function divergence after 4 terms (eps=2.16148)
greenPwc: CollocationGreenPwc(val=-0.001625878)
greenMpole: spo=(600,1200,100000) spc=(600,1200,100400) cnt=18 convRadius=1341.64 gs0332
greenMpole( 0,  2): value= 0.000660948 term= 0.000660948 eps=1.000000
greenMpole( 1, 11): value= 0.001319059 term= 0.000658111 eps=0.498925
greenMpole( 2, 29): value= 0.001316976 term=-0.000002083 eps=0.001582
greenMpole( 3, 53): value= 0.001317550 term= 0.000000574 eps=0.000435
greenPwc: CollocationGreenPwc(val= 0.001317550) val=-0.000154164
greenPwc: d=632.456 r=4800 sp1=(600,1200,100400) sp2=(0,1200,100200) B-F
greenMpole: spo=(600,1200,100400) spc=(0,1200,100200) cnt=19 convRadius=1216.55 gs0333
     ... (see A-B 2)
greenPwc: CollocationGreenPwc(val= 0.196255082)
greenMpole: spo=(0,1200,100200) spc=(600,1200,100400) cnt=20 convRadius=1341.64 gs0333
     ... (see A-B 1)
greenPwc: CollocationGreenPwc(val= 0.310064533) val= 0.253159807
greenPwc: d=0 r=2400 sp1=(600,2400,100200) sp2=(600,2400,100200) C-C
greenMpole: spo=(600,2400,100200) spc=(600,2400,100200) cnt=21 convRadius=632.456 gs0333
greenMpole( 0,  2): value= 4.663004836 term= 4.663004836 eps=1.000000
greenMpole( 1,  8): value= 2.590177684 term=-2.072827152 eps=0.800264
greenMpole( 2, 21): value= 2.600812526 term= 0.010634842 eps=0.004089
greenMpole( 3, 37): value= 2.594261271 term=-0.006551256 eps=0.002525
greenMpole( 4, 56): value= 2.600049557 term= 0.005788286 eps=0.002226
greenMpole( 5, 78): value= 2.594065687 term=-0.005983870 eps=0.002307
greenMpole: Greens function divergence after 6 terms (eps=0.00222622)
greenPwc: CollocationGreenPwc(val= 2.594065687)
greenPwc: d=2400 r=2400 sp1=(600,2400,100200) sp2=(600,0,100200) C-D
greenMpole: spo=(600,2400,100200) spc=(600,0,100200) cnt=22 convRadius=632.456 gs0333
greenMpole( 0,  2): value= 0.406854479 term= 0.406854479 eps=1.000000
greenMpole( 1,  8): value=-0.001172241 term=-0.408026721 eps=348.074051
greenMpole( 2, 21): value= 0.009459757 term= 0.010631998 eps=1.123919
greenMpole( 3, 37): value= 0.002908893 term=-0.006550864 eps=2.252013
greenMpole: Greens function divergence after 4 terms (eps=1.12392)
greenPwc: CollocationGreenPwc(val= 0.002908893)
greenMpole: spo=(600,0,100200) spc=(600,2400,100200) cnt=23 convRadius=632.456 gs0333
     ... (see C-D 1)
greenPwc: CollocationGreenPwc(val= 0.002908893) val= 0.002908893
greenPwc: d=1216.55 r=3600 sp1=(600,2400,100200) sp2=(600,1200,100000) C-E
greenMpole: spo=(600,2400,100200) spc=(600,1200,100000) cnt=24 convRadius=1341.64 gs0323
greenMpole( 0,  2): value= 1.085528035 term= 1.085528035 eps=1.000000
greenMpole( 1, 11): value=-0.005983384 term=-1.091511419 eps=182.423756
greenMpole( 2, 29): value= 0.004723115 term= 0.010706499 eps=2.266830
greenMpole( 3, 53): value=-0.001855788 term=-0.006578903 eps=3.545072
greenMpole: Greens function divergence after 4 terms (eps=2.26683)
greenPwc: CollocationGreenPwc(val=-0.001855788)
greenMpole: spo=(600,1200,100000) spc=(600,2400,100200) cnt=25 convRadius=632.456 gs0332
greenMpole( 0,  2): value= 0.000391914 term= 0.000391914 eps=1.000000
greenMpole( 1, 11): value= 0.000781257 term= 0.000389343 eps=0.498354
greenMpole( 2, 29): value= 0.000779185 term=-0.000002072 eps=0.002660
greenMpole( 3, 53): value= 0.000779755 term= 0.000000570 eps=0.000732
greenPwc: CollocationGreenPwc(val= 0.000779755) val=-0.000538016
greenPwc: d=1341.64 r=3600 sp1=(600,2400,100200) sp2=(0,1200,100200) C-F
greenMpole: spo=(600,2400,100200) spc=(0,1200,100200) cnt=26 convRadius=1216.55 gs0333
     ... (see A-C 2)
greenPwc: CollocationGreenPwc(val= 0.066216369)
greenMpole: spo=(0,1200,100200) spc=(600,2400,100200) cnt=27 convRadius=632.456 gs0333
     ... (see A-C 1)
greenPwc: CollocationGreenPwc(val= 0.027595320) val= 0.046905845
greenPwc: d=0 r=2400 sp1=(600,0,100200) sp2=(600,0,100200) D-D
greenMpole: spo=(600,0,100200) spc=(600,0,100200) cnt=28 convRadius=632.456 gs0333
     ... (see C-C)
greenPwc: CollocationGreenPwc(val= 2.594065687)
greenPwc: d=1216.55 r=3600 sp1=(600,0,100200) sp2=(600,1200,100000) D-E
greenMpole: spo=(600,0,100200) spc=(600,1200,100000) cnt=29 convRadius=1341.64 gs0323
     ... (see C-E 1)
greenPwc: CollocationGreenPwc(val=-0.001855788)
greenMpole: spo=(600,1200,100000) spc=(600,0,100200) cnt=30 convRadius=632.456 gs0332
     ... (see C-E 2)
greenPwc: CollocationGreenPwc(val= 0.000779755) val=-0.000538016
greenPwc: d=1341.64 r=3600 sp1=(600,0,100200) sp2=(0,1200,100200) D-F
greenMpole: spo=(600,0,100200) spc=(0,1200,100200) cnt=31 convRadius=1216.55 gs0333
     ... (see A-C 2)
greenPwc: CollocationGreenPwc(val= 0.066216369)
greenMpole: spo=(0,1200,100200) spc=(600,0,100200) cnt=32 convRadius=632.456 gs0333
     ... (see A-C 1)
greenPwc: CollocationGreenPwc(val= 0.027595320) val= 0.046905845
greenPwc: d=0 r=4800 sp1=(600,1200,100000) sp2=(600,1200,100000) E-E
greenMpole: spo=(600,1200,100000) spc=(600,1200,100000) cnt=33 convRadius=1341.64 gs0322
greenMpole( 0,  2): value= 0.001000025 term= 0.001000025 eps=1.000000
greenMpole( 1, 12): value= 0.001996867 term= 0.000996842 eps=0.499203
greenMpole( 2, 36): value= 0.001994805 term=-0.000002062 eps=0.001034
greenMpole( 3, 73): value= 0.001995372 term= 0.000000567 eps=0.000284
greenPwc: CollocationGreenPwc(val= 0.001995372)
greenPwc: d=632.456 r=4800 sp1=(600,1200,100000) sp2=(0,1200,100200) E-F
greenMpole: spo=(600,1200,100000) spc=(0,1200,100200) cnt=34 convRadius=1216.55 gs0332
     ... (see A-E 2)
greenPwc: CollocationGreenPwc(val= 0.001145278)
greenMpole: spo=(0,1200,100200) spc=(600,1200,100000) cnt=35 convRadius=1341.64 gs0323
     ... (see A-E 1)
greenPwc: CollocationGreenPwc(val=-0.001679354) val=-0.000267038
greenPwc: d=0 r=4800 sp1=(0,1200,100200) sp2=(0,1200,100200) F-F
greenMpole: spo=(0,1200,100200) spc=(0,1200,100200) cnt=36 convRadius=1216.55 gs0333
     ... (see A-A)
greenPwc: CollocationGreenPwc(val= 1.349858346)
space3d: Warning: maximum error not reached for 83.3% of the Greens functions.

overall resource utilization:
        memory allocation  : 0.192 Mbyte
	user time          :         0.0
	system time        :         0.1
-------------------------------------------------------------------------------

/* Date: 10-Apr-02 10:07:19 GMT */
network single2 ()
{
    cap 140.3211e-18 (SUBSTR, GND);
}

Note: Total number of terms is 2471
-------------------------------------------------------------------------------
.C0
.SK
.HU "APPENDIX B -- Results diff: merge_images=0"
.C8
8c8
< greenInit: use_mean_green_values=0 merge_images=1
> greenInit: use_mean_green_values=0 merge_images=0
26,29c26,29
< greenMpole( 5, 78): value= 1.349858346 term=-0.005983864 eps=0.004433
> greenMpole( 5,212): value= 1.349858346 term=-0.005983864 eps=0.004433
36,39c36,39
< greenMpole( 5, 78): value= 0.310064533 term=-0.005980692 eps=0.019289
> greenMpole( 5,212): value= 0.310064533 term=-0.005980692 eps=0.019289
45,48c45,48
< greenMpole( 5, 78): value= 0.196255082 term=-0.005980694 eps=0.030474
> greenMpole( 5,212): value= 0.196255082 term=-0.005980694 eps=0.030474
55,58c55,58
< greenMpole( 5, 78): value= 0.027595320 term=-0.005983840 eps=0.216843
> greenMpole( 5,212): value= 0.027595320 term=-0.005983840 eps=0.216843
64,67c64,67
< greenMpole( 5, 78): value= 0.066216369 term=-0.005983834 eps=0.090368
> greenMpole( 5,212): value= 0.066216369 term=-0.005983834 eps=0.090368
74,77c74,77
< greenMpole( 5, 78): value= 0.027595320 term=-0.005983840 eps=0.216843
> greenMpole( 5,212): value= 0.027595320 term=-0.005983840 eps=0.216843
83,86c83,86
< greenMpole( 5, 78): value= 0.066216369 term=-0.005983834 eps=0.090368
> greenMpole( 5,212): value= 0.066216369 term=-0.005983834 eps=0.090368
92,94c92,94
< greenMpole( 3, 53): value=-0.001679354 term=-0.006578977 eps=3.917563
> greenMpole( 3,102): value=-0.001679354 term=-0.006578977 eps=3.917563
99,101c99,101
< greenMpole( 3, 53): value= 0.001145278 term= 0.000000570 eps=0.000498
> greenMpole( 3,102): value= 0.001145278 term= 0.000000570 eps=0.000498
107,110c107,110
< greenMpole( 5, 78): value= 0.027281917 term=-0.005983840 eps=0.219334
> greenMpole( 5,212): value= 0.027281917 term=-0.005983840 eps=0.219334
116,119c116,119
< greenMpole( 5, 78): value= 0.027281917 term=-0.005983840 eps=0.219334
> greenMpole( 5,212): value= 0.027281917 term=-0.005983840 eps=0.219334
126,129c126,129
< greenMpole( 5, 78): value= 1.062218540 term=-0.005977536 eps=0.005627
> greenMpole( 5,212): value= 1.062218540 term=-0.005977536 eps=0.005627
136,139c136,139
< greenMpole( 5, 78): value= 0.067007170 term=-0.005980682 eps=0.089254
> greenMpole( 5,212): value= 0.067007170 term=-0.005980682 eps=0.089254
145,148c145,148
< greenMpole( 5, 78): value= 0.268358231 term=-0.005980674 eps=0.022286
> greenMpole( 5,212): value= 0.268358231 term=-0.005980674 eps=0.022286
155,158c155,158
< greenMpole( 5, 78): value= 0.067007170 term=-0.005980682 eps=0.089254
> greenMpole( 5,212): value= 0.067007170 term=-0.005980682 eps=0.089254
164,167c164,167
< greenMpole( 5, 78): value= 0.268358231 term=-0.005980674 eps=0.022286
> greenMpole( 5,212): value= 0.268358231 term=-0.005980674 eps=0.022286
173,175c173,175
< greenMpole( 3, 53): value=-0.001625878 term=-0.006593781 eps=4.055520
> greenMpole( 3,102): value=-0.001625878 term=-0.006593781 eps=4.055520
180,182c180,182
< greenMpole( 3, 53): value= 0.001317550 term= 0.000000574 eps=0.000435
> greenMpole( 3,102): value= 0.001317550 term= 0.000000574 eps=0.000435
188,191c188,191
< greenMpole( 5, 78): value= 0.196255082 term=-0.005980694 eps=0.030474
> greenMpole( 5,212): value= 0.196255082 term=-0.005980694 eps=0.030474
197,200c197,200
< greenMpole( 5, 78): value= 0.310064533 term=-0.005980692 eps=0.019289
> greenMpole( 5,212): value= 0.310064533 term=-0.005980692 eps=0.019289
207,210c207,210
< greenMpole( 5, 78): value= 2.594065687 term=-0.005983870 eps=0.002307
> greenMpole( 5,212): value= 2.594065687 term=-0.005983870 eps=0.002307
217,218c217,218
< greenMpole( 3, 37): value= 0.002908893 term=-0.006550864 eps=2.252013
> greenMpole( 3, 62): value= 0.002908893 term=-0.006550864 eps=2.252013
224,225c224,225
< greenMpole( 3, 37): value= 0.002908893 term=-0.006550864 eps=2.252013
> greenMpole( 3, 62): value= 0.002908893 term=-0.006550864 eps=2.252013
231,233c231,233
< greenMpole( 3, 53): value=-0.001855788 term=-0.006578903 eps=3.545072
> greenMpole( 3,102): value=-0.001855788 term=-0.006578903 eps=3.545072
238,240c238,240
< greenMpole( 3, 53): value= 0.000779755 term= 0.000000570 eps=0.000732
> greenMpole( 3,102): value= 0.000779755 term= 0.000000570 eps=0.000732
246,249c246,249
< greenMpole( 5, 78): value= 0.066216369 term=-0.005983834 eps=0.090368
> greenMpole( 5,212): value= 0.066216369 term=-0.005983834 eps=0.090368
255,258c255,258
< greenMpole( 5, 78): value= 0.027595320 term=-0.005983840 eps=0.216843
> greenMpole( 5,212): value= 0.027595320 term=-0.005983840 eps=0.216843
265,268c265,268
< greenMpole( 5, 78): value= 2.594065687 term=-0.005983870 eps=0.002307
> greenMpole( 5,212): value= 2.594065687 term=-0.005983870 eps=0.002307
274,276c274,276
< greenMpole( 3, 53): value=-0.001855788 term=-0.006578903 eps=3.545072
> greenMpole( 3,102): value=-0.001855788 term=-0.006578903 eps=3.545072
281,283c281,283
< greenMpole( 3, 53): value= 0.000779755 term= 0.000000570 eps=0.000732
> greenMpole( 3,102): value= 0.000779755 term= 0.000000570 eps=0.000732
289,292c289,292
< greenMpole( 5, 78): value= 0.066216369 term=-0.005983834 eps=0.090368
> greenMpole( 5,212): value= 0.066216369 term=-0.005983834 eps=0.090368
298,301c298,301
< greenMpole( 5, 78): value= 0.027595320 term=-0.005983840 eps=0.216843
> greenMpole( 5,212): value= 0.027595320 term=-0.005983840 eps=0.216843
308,309c308,309
< greenMpole( 3, 73): value= 0.001995372 term= 0.000000567 eps=0.000284
> greenMpole( 3,102): value= 0.001995372 term= 0.000000567 eps=0.000284
314,316c314,316
< greenMpole( 3, 53): value= 0.001145278 term= 0.000000570 eps=0.000498
> greenMpole( 3,102): value= 0.001145278 term= 0.000000570 eps=0.000498
320,322c320,322
< greenMpole( 3, 53): value=-0.001679354 term=-0.006578977 eps=3.917563
> greenMpole( 3,102): value=-0.001679354 term=-0.006578977 eps=3.917563
329,332c329,332
< greenMpole( 5, 78): value= 1.349858346 term=-0.005983864 eps=0.004433
> greenMpole( 5,212): value= 1.349858346 term=-0.005983864 eps=0.004433
.C0
.SK
.HU "APPENDIX C -- Results diff: use_lowest_medium=0"
.C8
7c7
< greenInit: use_lowest_medium=1 use_old_images=0
> greenInit: use_lowest_medium=0 use_old_images=0
90c90
< greenMpole: spo=(1200,1200,100200) spc=(600,1200,100000) cnt=8 convRadius=1341.64 gs0323
> greenMpole: spo=(1200,1200,100200) spc=(600,1200,100000) cnt=8 convRadius=1341.64 gs0333
92,102c92,103
< greenMpole( 3, 53): value=-0.001679354 term=-0.006578977 eps=3.917563
< greenMpole: Greens function divergence after 4 terms (eps=2.18528)
< greenPwc: CollocationGreenPwc(val=-0.001679354)
< greenMpole: spo=(600,1200,100000) spc=(1200,1200,100200) cnt=9 convRadius=1216.55 gs0332
< greenMpole( 3, 53): value= 0.001145278 term= 0.000000570 eps=0.000498
< greenPwc: CollocationGreenPwc(val= 0.001145278) val=-0.000267038
---
> greenMpole( 3, 37): value=-0.001670751 term=-0.006557698 eps=3.924999
> greenMpole: Greens function divergence after 4 terms (eps=2.18037)
> greenPwc: CollocationGreenPwc(val=-0.001670751)
> greenMpole: spo=(600,1200,100000) spc=(1200,1200,100200) cnt=9 convRadius=1216.55 gs0333
> greenMpole( 3, 37): value=-0.001783233 term=-0.006557708 eps=3.677427
> greenMpole: Greens function divergence after 4 terms (eps=2.23175)
> greenPwc: CollocationGreenPwc(val=-0.001783233) val=-0.001726992
171c172
< greenMpole: spo=(600,1200,100400) spc=(600,1200,100000) cnt=17 convRadius=1341.64 gs0323
> greenMpole: spo=(600,1200,100400) spc=(600,1200,100000) cnt=17 convRadius=1341.64 gs0333
173,183c174,185
< greenMpole( 3, 53): value=-0.001625878 term=-0.006593781 eps=4.055520
< greenMpole: Greens function divergence after 4 terms (eps=2.16148)
< greenPwc: CollocationGreenPwc(val=-0.001625878)
< greenMpole: spo=(600,1200,100000) spc=(600,1200,100400) cnt=18 convRadius=1341.64 gs0332
< greenMpole( 3, 53): value= 0.001317550 term= 0.000000574 eps=0.000435
< greenPwc: CollocationGreenPwc(val= 0.001317550) val=-0.000154164
---
> greenMpole( 3, 37): value=-0.001608672 term=-0.006551221 eps=4.072441
> greenMpole: Greens function divergence after 4 terms (eps=2.15164)
> greenPwc: CollocationGreenPwc(val=-0.001608672)
> greenMpole: spo=(600,1200,100000) spc=(600,1200,100400) cnt=18 convRadius=1341.64 gs0333
> greenMpole( 3, 37): value=-0.001608672 term=-0.006551221 eps=4.072441
> greenMpole: Greens function divergence after 4 terms (eps=2.15164)
> greenPwc: CollocationGreenPwc(val=-0.001608672) val=-0.001608672
229c231
< greenMpole: spo=(600,2400,100200) spc=(600,1200,100000) cnt=24 convRadius=1341.64 gs0323
> greenMpole: spo=(600,2400,100200) spc=(600,1200,100000) cnt=24 convRadius=1341.64 gs0333
231,241c233,244
< greenMpole( 3, 53): value=-0.001855788 term=-0.006578903 eps=3.545072
< greenMpole: Greens function divergence after 4 terms (eps=2.26683)
< greenPwc: CollocationGreenPwc(val=-0.001855788)
< greenMpole: spo=(600,1200,100000) spc=(600,2400,100200) cnt=25 convRadius=632.456 gs0332
< greenMpole( 3, 53): value= 0.000779755 term= 0.000000570 eps=0.000732
< greenPwc: CollocationGreenPwc(val= 0.000779755) val=-0.000538016
---
> greenMpole( 3, 37): value=-0.001847185 term=-0.006557624 eps=3.550063
> greenMpole: Greens function divergence after 4 terms (eps=2.26196)
> greenPwc: CollocationGreenPwc(val=-0.001847185)
> greenMpole: spo=(600,1200,100000) spc=(600,2400,100200) cnt=25 convRadius=632.456 gs0333
> greenMpole( 3, 37): value=-0.002148743 term=-0.006557658 eps=3.051858
> greenMpole: Greens function divergence after 4 terms (eps=2.41671)
> greenPwc: CollocationGreenPwc(val=-0.002148743) val=-0.001997964
272c275
< greenMpole: spo=(600,0,100200) spc=(600,1200,100000) cnt=29 convRadius=1341.64 gs0323
> greenMpole: spo=(600,0,100200) spc=(600,1200,100000) cnt=29 convRadius=1341.64 gs0333
274,284c277,288
< greenMpole( 3, 53): value=-0.001855788 term=-0.006578903 eps=3.545072
< greenMpole: Greens function divergence after 4 terms (eps=2.26683)
< greenPwc: CollocationGreenPwc(val=-0.001855788)
< greenMpole: spo=(600,1200,100000) spc=(600,0,100200) cnt=30 convRadius=632.456 gs0332
< greenMpole( 3, 53): value= 0.000779755 term= 0.000000570 eps=0.000732
< greenPwc: CollocationGreenPwc(val= 0.000779755) val=-0.000538016
---
> greenMpole( 3, 37): value=-0.001847185 term=-0.006557624 eps=3.550063
> greenMpole: Greens function divergence after 4 terms (eps=2.26196)
> greenPwc: CollocationGreenPwc(val=-0.001847185)
> greenMpole: spo=(600,1200,100000) spc=(600,0,100200) cnt=30 convRadius=632.456 gs0333
> greenMpole( 3, 37): value=-0.002148743 term=-0.006557658 eps=3.051858
> greenMpole: Greens function divergence after 4 terms (eps=2.41671)
> greenPwc: CollocationGreenPwc(val=-0.002148743) val=-0.001997964
305,310c309,315
< greenMpole: spo=(600,1200,100000) spc=(600,1200,100000) cnt=33 convRadius=1341.64 gs0322
< greenMpole( 3, 73): value= 0.001995372 term= 0.000000567 eps=0.000284
< greenPwc: CollocationGreenPwc(val= 0.001995372)
---
> greenMpole: spo=(600,1200,100000) spc=(600,1200,100000) cnt=33 convRadius=1341.64 gs0333
> greenMpole( 3, 37): value=-0.000935439 term=-0.006564234 eps=7.017278
> greenMpole: Greens function divergence after 4 terms (eps=1.89677)
> greenPwc: CollocationGreenPwc(val=-0.000935439)
312,318c317,324
< greenMpole: spo=(600,1200,100000) spc=(0,1200,100200) cnt=34 convRadius=1216.55 gs0332
< greenMpole( 3, 53): value= 0.001145278 term= 0.000000570 eps=0.000498
< greenPwc: CollocationGreenPwc(val= 0.001145278)
< greenMpole: spo=(0,1200,100200) spc=(600,1200,100000) cnt=35 convRadius=1341.64 gs0323
---
> greenMpole: spo=(600,1200,100000) spc=(0,1200,100200) cnt=34 convRadius=1216.55 gs0333
> greenMpole( 3, 37): value=-0.001783233 term=-0.006557708 eps=3.677427
> greenMpole: Greens function divergence after 4 terms (eps=2.23175)
> greenPwc: CollocationGreenPwc(val=-0.001783233)
> greenMpole: spo=(0,1200,100200) spc=(600,1200,100000) cnt=35 convRadius=1341.64 gs0333
320,324c326,330
< greenMpole( 3, 53): value=-0.001679354 term=-0.006578977 eps=3.917563
< greenMpole: Greens function divergence after 4 terms (eps=2.18528)
< greenPwc: CollocationGreenPwc(val=-0.001679354) val=-0.000267038
---
> greenMpole( 3, 37): value=-0.001670751 term=-0.006557698 eps=3.924999
> greenMpole: Greens function divergence after 4 terms (eps=2.18037)
> greenPwc: CollocationGreenPwc(val=-0.001670751) val=-0.001726992
335c341,343
< space3d: Warning: maximum error not reached for 83.3% of the Greens functions.
---
> space3d: domain error(s) in sqrt
> space3d: 3 domain errors in sqrt
> space3d: Warning: maximum error not reached for 100.0% of the Greens functions.
346c354
<         memory allocation  : 0.192 Mbyte
>         memory allocation  : 0.184 Mbyte
357c365
<     cap 140.3211e-18 (SUBSTR, GND);
>     cap 000m (SUBSTR, GND);


-------------------------------------------------------------------------------
Note: Also for min_divergence_term=10 no good result is reached.
-------------------------------------------------------------------------------
.C0
.SK
.HU "APPENDIX D -- Results diff: use_mean_green_values=1"
.C8
8c8
< greenInit: use_mean_green_values=0 merge_images=1
> greenInit: use_mean_green_values=1 merge_images=1
31c31
< greenPwc: CollocationGreenPwc(val= 1.349858346)
> greenPwc: CollocationGreenPwc(val= 1.353285122)
41c41
< greenPwc: CollocationGreenPwc(val= 0.310064533)
> greenPwc: CollocationGreenPwc(val= 0.313489103)
50c50
< greenPwc: CollocationGreenPwc(val= 0.196255082) val= 0.253159807
> greenPwc: CollocationGreenPwc(val= 0.199679654) val= 0.256584378
60c60
< greenPwc: CollocationGreenPwc(val= 0.027595320)
> greenPwc: CollocationGreenPwc(val= 0.031022069)
69c69
< greenPwc: CollocationGreenPwc(val= 0.066216369) val= 0.046905845
> greenPwc: CollocationGreenPwc(val= 0.069643111) val= 0.050332590
79c79
< greenPwc: CollocationGreenPwc(val= 0.027595320)
> greenPwc: CollocationGreenPwc(val= 0.031022069)
88c88
< greenPwc: CollocationGreenPwc(val= 0.066216369) val= 0.046905845
> greenPwc: CollocationGreenPwc(val= 0.069643111) val= 0.050332590
96c96
< greenPwc: CollocationGreenPwc(val=-0.001679354)
> greenPwc: CollocationGreenPwc(val= 0.004854413)
102c102
< greenPwc: CollocationGreenPwc(val= 0.001145278) val=-0.000267038
> greenPwc: CollocationGreenPwc(val= 0.001144993) val= 0.002999703
112c112
< greenPwc: CollocationGreenPwc(val= 0.027281917)
> greenPwc: CollocationGreenPwc(val= 0.030708666)
121c121
< greenPwc: CollocationGreenPwc(val= 0.027281917) val= 0.027281917
> greenPwc: CollocationGreenPwc(val= 0.030708666) val= 0.030708666
131c131
< greenPwc: CollocationGreenPwc(val= 1.062218540)
> greenPwc: CollocationGreenPwc(val= 1.065640922)
141c141
< greenPwc: CollocationGreenPwc(val= 0.067007170)
> greenPwc: CollocationGreenPwc(val= 0.070431729)
150c150
< greenPwc: CollocationGreenPwc(val= 0.268358231) val= 0.167682701
> greenPwc: CollocationGreenPwc(val= 0.271782780) val= 0.171107255
160c160
< greenPwc: CollocationGreenPwc(val= 0.067007170)
> greenPwc: CollocationGreenPwc(val= 0.070431729)
169c169
< greenPwc: CollocationGreenPwc(val= 0.268358231) val= 0.167682701
> greenPwc: CollocationGreenPwc(val= 0.271782780) val= 0.171107255
177c177
< greenPwc: CollocationGreenPwc(val=-0.001625878)
> greenPwc: CollocationGreenPwc(val= 0.004924479)
183c183
< greenPwc: CollocationGreenPwc(val= 0.001317550) val=-0.000154164
> greenPwc: CollocationGreenPwc(val= 0.001317263) val= 0.003120871
193c193
< greenPwc: CollocationGreenPwc(val= 0.196255082)
> greenPwc: CollocationGreenPwc(val= 0.199679654)
202c202
< greenPwc: CollocationGreenPwc(val= 0.310064533) val= 0.253159807
> greenPwc: CollocationGreenPwc(val= 0.313489103) val= 0.256584378
212c212
< greenPwc: CollocationGreenPwc(val= 2.594065687)
> greenPwc: CollocationGreenPwc(val= 2.597492470)
220c220
< greenPwc: CollocationGreenPwc(val= 0.002908893)
> greenPwc: CollocationGreenPwc(val= 0.009321237)
227c227
< greenPwc: CollocationGreenPwc(val= 0.002908893) val= 0.002908893
> greenPwc: CollocationGreenPwc(val= 0.009321237) val= 0.009321237
235c235
< greenPwc: CollocationGreenPwc(val=-0.001855788)
> greenPwc: CollocationGreenPwc(val= 0.004670605)
241c241
< greenPwc: CollocationGreenPwc(val= 0.000779755) val=-0.000538016
> greenPwc: CollocationGreenPwc(val= 0.000779470) val= 0.002725038
251c251
< greenPwc: CollocationGreenPwc(val= 0.066216369)
> greenPwc: CollocationGreenPwc(val= 0.069643111)
260c260
< greenPwc: CollocationGreenPwc(val= 0.027595320) val= 0.046905845
> greenPwc: CollocationGreenPwc(val= 0.031022069) val= 0.050332590
270c270
< greenPwc: CollocationGreenPwc(val= 2.594065687)
> greenPwc: CollocationGreenPwc(val= 2.597492470)
278c278
< greenPwc: CollocationGreenPwc(val=-0.001855788)
> greenPwc: CollocationGreenPwc(val= 0.004670605)
284c284
< greenPwc: CollocationGreenPwc(val= 0.000779755) val=-0.000538016
> greenPwc: CollocationGreenPwc(val= 0.000779470) val= 0.002725038
294c294
< greenPwc: CollocationGreenPwc(val= 0.066216369)
> greenPwc: CollocationGreenPwc(val= 0.069643111)
303c303
< greenPwc: CollocationGreenPwc(val= 0.027595320) val= 0.046905845
> greenPwc: CollocationGreenPwc(val= 0.031022069) val= 0.050332590
310c310
< greenPwc: CollocationGreenPwc(val= 0.001995372)
> greenPwc: CollocationGreenPwc(val= 0.001995088)
317c317
< greenPwc: CollocationGreenPwc(val= 0.001145278)
> greenPwc: CollocationGreenPwc(val= 0.001144993)
324c324,333
< greenPwc: CollocationGreenPwc(val=-0.001679354) val=-0.000267038
> greenPwc: CollocationGreenPwc(val= 0.004854413) val= 0.002999703
> space3d: Inconsistent value in influence matrix, probably due to too large
> 	a mesh granularity (large differences in element sizes).
> 	Extraction results will be incorrect.
> 	It concerns the following mesh points (coordinates are in meters,
> 	sizes in square meters):
> 	  point 1: mask=cmf x=1.500000e-06 y=3.000000e-06 z=2.500000e-04
> 	  connected to element with size 1.800000e-11
> 	  point 2: mask=cmf x=1.500000e-06 y=3.000000e-06 z=2.500000e-04
> 	  connected to element with size 6.000000e-12
334c343
< greenPwc: CollocationGreenPwc(val= 1.349858346)
> greenPwc: CollocationGreenPwc(val= 1.353285122)
357c366
<     cap 140.3211e-18 (SUBSTR, GND);
>     cap 139.5968e-18 (SUBSTR, GND);
.C0
