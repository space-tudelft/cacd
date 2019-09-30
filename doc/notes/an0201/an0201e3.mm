.T= "Sub3D Extraction Example3"
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
SUBSTRATE 3D EXTRACTION
EXAMPLE3 (single/duo2)
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
Report ET-CAS 02-01-3
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
.H 1 "EXAMPLE CELL: single"
.H 2 "single.ldm"
.C1
:: lambda = 0.01 micron
ms single
term cmf 125 154 278 313 a
box cmf 0 300 0 600
me
.C0
.F+
.PSPIC an0201/e3_f1.ps 14c
.F-
.H 2 "results"
For the above configuration see listings in APPENDIX A to G.
.P
The following results are for grounded substrate:
.DS C
.TS
box;
c|s|s|s
r|r|l|l.
neumann_simulation_ratio=1e4
_
spB-cx	spA-cx	res trunc.	res mean
_
--	600	12.72083k	12.71818k
50	650	12.07127k	12.06875k
100	700	12.06327k	12.06083k
150	750	12.07292k	12.07058k
200	800	12.08414k	12.08191k
250	850	12.09198k	12.08986k
300	900	12.09325k	12.09192k
.TE
.DE
.P
.DS C
.TS
box;
c|s|s
c|l|l.
spB-cx=250 spA-cx=850
_
neumann_sim_ratio	res trunc.	res mean
_
1e5	12.09306k	12.09095k
1e4	12.09198k	12.08986k
1e3	12.08112k	12.07901k
1e2	11.97360k	11.97158k
1e1	10.99418k	10.99338k
.TE
.DE
.H 1 "EXAMPLE SETUP"
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

sublayers:
    epi        10      0
 #  epi2       40   -100
    substrate  1e5  -250 
 #  metalization 1  -250 
.C0
.H 2 "space.green.p"
.C1
#min_divergence_term   10
#sub3d.use_old_images           on
#sub3d.use_lowest_medium        off
#sub3d.use_mean_green_values    on
#sub3d.merge_images             off

#sub3d.green_eps          0.002
#sub3d.mp_min_dist       22
#sub3d.max_green_terms  100
sub3d.max_be_area               inf
sub3d.be_window                 inf
sub3d.neumann_simulation_ratio  1e4

debug.print_green_init
debug.print_green_terms
#debug.print_green_gterms
.C0
.H 2 "execution statement"
.C1
space3d -vB -E space.green.t -P space.green.p single
.C0
.H 1 "EXAMPLE SETUP-B"
.F+
.PSPIC an0201/e3_f2.ps 14c
.F-
.H 2 "results"
The following results are for grounded substrate (nLayers=2),
.br
for different max_be_area values:
.DS C
.TS
box;
c|s|s|s|s
r|r|l|r|r.
neumann_simulation_ratio=1e4
_
max_be_area	dimension	res trunc.	# calc.	# needed
_
1	32	10.51425k	1024	-
2	16	10.72377k	256	-
4	8	11.08130k	64	8
6	4	11.47751k	16	4
10	2	12.05888k	4	2
inf	1	12.72083k	1	1
.TE
.DE
.H 1 "EXAMPLE SETUP-C: duo2"
.F+
.PSPIC an0201/e3_f3.ps 14c
.F-
.H 2 "results"
The following results are for grounded substrate (nLayers=2),
.br
for different distance A-B values:
.DS C
.TS
box;
c|s|s
r|l|l.
neumann_simulation_ratio=1e4
_
distance A-B	res(1,2)	res(1,SUBSTR)
_
2400	60.78144k	15.27575k
3600	93.01417k	14.42918k
4800	125.6712k	13.99554k
6000	158.6319k	13.73445k
7200	192.1953k	13.55913k
8400	226.4985k	13.43303k
.TE
.DE
.C1

network duo2 ()
{
    res 226.4985k (1, 2);
    res 13.43303k (1, SUBSTR);
    res 13.43303k (2, SUBSTR);
}
.C0
.H 1 "APPENDICES"
.SP
.nr Hu 2 \" Level of this .HU heading
.HU "APPENDIX A -- Results: normal substrate, nLayers=1"
.C8
preprocessing single (phase 1 - flattening layout)
preprocessing single (phase 2 - removing overlap)
prepassing single for substrate resistance
greenInit: use_lowest_medium=1 use_old_images=0
greenInit: use_mean_green_values=0 merge_images=1
greenInit: use_multipoles=1 test_multipoles=0 min_divergence_term=30
greenInit: be_mode='collocation' FeModeGalerkin=0 FeModePwl=0
greenInit: reading dielectric specification
greenInit: i=1 m='SiO2' e=3.9 b=0
greenInit: i=2 m='air' e=0.001 b=250
greenInit: reading substrate specification
greenInit: i=1 m='epi' s=10 t=0
greenInit: greenType=2 nLayers=1 maxGreenTerms=1 collocationEps=0.001
strip 0 300 (add)
Schur dimension 2, maxorder 1
greenPwc: d=0 r=4800 sp1=(850,1200,0) sp2=(850,1200,0)
greenMpole: spo=(850,1200,0) spc=(850,1200,0) cnt=1 convRadius=1250 gs2111
greenMpole( 0,  1): value= 0.000488708 term= 0.000488708 eps=1.000000
greenPwc: CollocationGreenPwc(val= 0.000488708)
greenPwc: d=600 r=4800 sp1=(850,1200,0) sp2=(250,1200,0)
greenMpole: spo=(850,1200,0) spc=(250,1200,0) cnt=2 convRadius=1225.77 gs2111
greenMpole( 0,  1): value= 0.000246057 term= 0.000246057 eps=1.000000
greenPwc: CollocationGreenPwc(val= 0.000246057)
greenMpole: spo=(250,1200,0) spc=(850,1200,0) cnt=3 convRadius=1250 gs2111
greenMpole( 0,  1): value= 0.000251914 term= 0.000251914 eps=1.000000
greenPwc: CollocationGreenPwc(val= 0.000251914) val= 0.000248985
greenPwc: d=0 r=4800 sp1=(250,1200,0) sp2=(250,1200,0)
greenMpole: spo=(250,1200,0) spc=(250,1200,0) cnt=4 convRadius=1225.77 gs2111
greenMpole( 0,  1): value= 0.000544224 term= 0.000544224 eps=1.000000
greenPwc: CollocationGreenPwc(val= 0.000544224)
extracting single
extraction statistics for layout single:
        capacitances        : 0
        resistances         : 1
        nodes               : 2
        mos transistors     : 0
        bipolar vertical    : 0
        bipolar lateral     : 0
        substrate terminals : 1
        substrate nodes     : 1

overall resource utilization:
        memory allocation  : 0.188 Mbyte
	user time          :         0.0
	system time        :         0.0
	real time          :         2.6   2%

space3d: --- Finished ---

/* Date: 12-Apr-02 15:43:24 GMT */
network single (terminal a)
{
    res 12.13669k (a, SUBSTR);
}
.C0
.SK
.HU "APPENDIX B -- Results: normal substrate, nLayers=2"
.C8
prepassing single for substrate resistance
greenInit: use_lowest_medium=1 use_old_images=0
greenInit: use_mean_green_values=0 merge_images=1
greenInit: use_multipoles=1 test_multipoles=0 min_divergence_term=10
greenInit: be_mode='collocation' FeModeGalerkin=0 FeModePwl=0
greenInit: reading substrate specification
greenInit: i=1 m='epi' s=10 t=0
greenInit: i=2 m='substrate' s=100000 t=-250
greenInit: greenType=2 nLayers=2 maxGreenTerms=500 collocationEps=0.001
strip 0 300 (add)
Schur dimension 2, maxorder 1
greenPwc: d=0 r=4800 sp1=(850,1200,0) sp2=(850,1200,0) A-A
greenMpole: spo=(850,1200,0) spc=(850,1200,0) cnt=1 convRadius=1250 gs2211
greenMpole( 0,  1): value= 0.000488708 term= 0.000488708 eps=1.000000
greenMpole( 1,  2): value= 0.000486709 term=-0.000002000 eps=0.004108
greenMpole( 2,  3): value= 0.000487708 term= 0.000001000 eps=0.002050
greenMpole( 3,  4): value= 0.000487042 term=-0.000000666 eps=0.001368
greenMpole( 4,  5): value= 0.000487541 term= 0.000000500 eps=0.001025
greenMpole( 5,  6): value= 0.000487142 term=-0.000000400 eps=0.000820
greenPwc: CollocationGreenPwc(val= 0.000487142)
greenPwc: d=600 r=4800 sp1=(850,1200,0) sp2=(250,1200,0) A-B
greenMpole: spo=(850,1200,0) spc=(250,1200,0) cnt=2 convRadius=1225.77 gs2211
greenMpole( 0,  1): value= 0.000246057 term= 0.000246057 eps=1.000000
greenMpole( 1,  2): value= 0.000244057 term=-0.000002000 eps=0.008193
greenMpole( 2,  3): value= 0.000245057 term= 0.000001000 eps=0.004079
greenMpole( 3,  4): value= 0.000244390 term=-0.000000666 eps=0.002726
greenMpole( 4,  5): value= 0.000244890 term= 0.000000500 eps=0.002040
greenMpole( 5,  6): value= 0.000244490 term=-0.000000400 eps=0.001634
greenMpole( 6,  7): value= 0.000244823 term= 0.000000333 eps=0.001360
greenMpole( 7,  8): value= 0.000244538 term=-0.000000285 eps=0.001167
greenMpole( 8,  9): value= 0.000244788 term= 0.000000250 eps=0.001020
greenMpole( 9, 10): value= 0.000244566 term=-0.000000222 eps=0.000907
greenPwc: CollocationGreenPwc(val= 0.000244566)
greenMpole: spo=(250,1200,0) spc=(850,1200,0) cnt=3 convRadius=1250 gs2211
greenMpole( 0,  1): value= 0.000251914 term= 0.000251914 eps=1.000000
greenMpole( 1,  2): value= 0.000249914 term=-0.000002000 eps=0.008001
greenMpole( 2,  3): value= 0.000250914 term= 0.000001000 eps=0.003984
greenMpole( 3,  4): value= 0.000250248 term=-0.000000666 eps=0.002662
greenMpole( 4,  5): value= 0.000250747 term= 0.000000500 eps=0.001992
greenMpole( 5,  6): value= 0.000250348 term=-0.000000400 eps=0.001596
greenMpole( 6,  7): value= 0.000250681 term= 0.000000333 eps=0.001328
greenMpole( 7,  8): value= 0.000250395 term=-0.000000285 eps=0.001139
greenMpole( 8,  9): value= 0.000250645 term= 0.000000250 eps=0.000996
greenPwc: CollocationGreenPwc(val= 0.000250645) val= 0.000247605
greenPwc: d=0 r=4800 sp1=(250,1200,0) sp2=(250,1200,0) B-B
greenMpole: spo=(250,1200,0) spc=(250,1200,0) cnt=4 convRadius=1225.77 gs2211
greenMpole( 0,  1): value= 0.000544224 term= 0.000544224 eps=1.000000
greenMpole( 1,  2): value= 0.000542225 term=-0.000002000 eps=0.003688
greenMpole( 2,  3): value= 0.000543224 term= 0.000001000 eps=0.001840
greenMpole( 3,  4): value= 0.000542558 term=-0.000000666 eps=0.001228
greenMpole( 4,  5): value= 0.000543058 term= 0.000000500 eps=0.000920
greenPwc: CollocationGreenPwc(val= 0.000543058)
extracting single

/* Date: 11-Apr-02 15:10:27 GMT */
network single (terminal a)
{
    res 12.09232k (a, SUBSTR);
}
.C0
.SK
.HU "APPENDIX C -- Results: grounded substrate, nLayers=2"
.C8
prepassing single for substrate resistance
greenInit: use_lowest_medium=1 use_old_images=0
greenInit: use_mean_green_values=0 merge_images=1
greenInit: use_multipoles=1 test_multipoles=0 min_divergence_term=10
greenInit: be_mode='collocation' FeModeGalerkin=0 FeModePwl=0
greenInit: reading substrate specification
greenInit: i=1 m='epi' s=10 t=0
greenInit: i=2 m='metalization' s=1 t=-250
greenInit: sub3d.neumann_simulation_ratio=10000
greenInit: i=1 e=10 b=0
greenInit: i=2 e=0.001 b=250
greenInit: greenType=3 nLayers=2 maxGreenTerms=500 collocationEps=0.001
strip 0 300 (add)
Schur dimension 2, maxorder 1
greenPwc: d=0 r=4800 sp1=(850,1200,100000) sp2=(850,1200,100000)
greenMpole: spo=(850,1200,100000) spc=(850,1200,100000) cnt=1 convRadius=1250 gs3222
greenMpole( 0,  3): value= 0.000487659 term= 0.000487659 eps=1.000000
greenMpole( 1,  5): value= 0.000487160 term=-0.000000500 eps=0.001026
greenMpole( 2,  7): value= 0.000487326 term= 0.000000167 eps=0.000342
greenPwc: CollocationGreenPwc(val= 0.000487326)
greenPwc: d=600 r=4800 sp1=(850,1200,100000) sp2=(250,1200,100000)
greenMpole: spo=(850,1200,100000) spc=(250,1200,100000) cnt=2 convRadius=1225.77 gs3222
greenMpole( 0,  3): value= 0.000245032 term= 0.000245032 eps=1.000000
greenMpole( 1,  5): value= 0.000244532 term=-0.000000500 eps=0.002044
greenMpole( 2,  7): value= 0.000244699 term= 0.000000167 eps=0.000681
greenPwc: CollocationGreenPwc(val= 0.000244699)
greenMpole: spo=(250,1200,100000) spc=(850,1200,100000) cnt=3 convRadius=1250 gs3222
greenMpole( 0,  3): value= 0.000250889 term= 0.000250889 eps=1.000000
greenMpole( 1,  5): value= 0.000250389 term=-0.000000500 eps=0.001996
greenMpole( 2,  7): value= 0.000250555 term= 0.000000167 eps=0.000665
greenPwc: CollocationGreenPwc(val= 0.000250555) val= 0.000247627
greenPwc: d=0 r=4800 sp1=(250,1200,100000) sp2=(250,1200,100000)
greenMpole: spo=(250,1200,100000) spc=(250,1200,100000) cnt=4 convRadius=1225.77 gs3222
greenMpole( 0,  3): value= 0.000543170 term= 0.000543170 eps=1.000000
greenMpole( 1,  5): value= 0.000542670 term=-0.000000500 eps=0.000921
greenPwc: CollocationGreenPwc(val= 0.000542670)
extracting single

/* Date: 11-Apr-02 15:48:27 GMT */
network single (terminal a)
{
    res 12.09198k (a, SUBSTR);
}
.C0
.HU "APPENDIX D -- Results: grounded substrate, nLayers=2 (new/mean diff)"
.C8
8c8
< greenInit: use_mean_green_values=0 merge_images=1
> greenInit: use_mean_green_values=1 merge_images=1
28c28
< greenPwc: CollocationGreenPwc(val= 0.000487326)
> greenPwc: CollocationGreenPwc(val= 0.000487243)
34c34
< greenPwc: CollocationGreenPwc(val= 0.000244699)
> greenPwc: CollocationGreenPwc(val= 0.000244616)
39c39
< greenPwc: CollocationGreenPwc(val= 0.000250555) val= 0.000247627
> greenPwc: CollocationGreenPwc(val= 0.000250472) val= 0.000247544
69c69
<     res 12.09198k (a, SUBSTR);
>     res 12.08986k (a, SUBSTR);
.C0
.SK
.HU "APPENDIX E -- Results: grounded substrate, nLayers=2 (new/old diff)"
.C8
7c7
< greenInit: use_lowest_medium=1 use_old_images=0
> greenInit: use_lowest_medium=1 use_old_images=1
9c9
< greenInit: use_multipoles=1 test_multipoles=0 min_divergence_term=10
> greenInit: use_multipoles=1 test_multipoles=0 min_divergence_term=30
25,28c25,168
< greenMpole( 2,  7): value= 0.000487326 term= 0.000000167 eps=0.000342
< greenPwc: CollocationGreenPwc(val= 0.000487326)
---
> greenMpole( 0,  2): value= 2.438540602 term= 2.438540602 eps=1.000000
> greenMpole( 1,  4): value=-0.002011812 term=-2.440552414 eps=1213.111439
> greenMpole( 2,  6): value= 0.001320157 term= 0.003331969 eps=2.523919
> greenMpole( 3,  8): value= 0.000070910 term=-0.001249247 eps=17.617343 <----
> greenMpole( 4, 10): value= 0.000737043 term= 0.000666133 eps=0.903791
> greenMpole( 5, 12): value= 0.000320793 term=-0.000416250 eps=1.297566 <----
> greenMpole( 6, 14): value= 0.000606164 term= 0.000285371 eps=0.470782
> greenMpole( 7, 16): value= 0.000398122 term=-0.000208042 eps=0.522557 <----
> greenMpole( 8, 18): value= 0.000556599 term= 0.000158476 eps=0.284723
> greenMpole( 9, 20): value= 0.000431824 term=-0.000124775 eps=0.288949 <----
> greenMpole(10, 22): value= 0.000532632 term= 0.000100808 eps=0.189264
> greenMpole(11, 24): value= 0.000449482 term=-0.000083150 eps=0.184991
> greenMpole(12, 26): value= 0.000519244 term= 0.000069762 eps=0.134354
  ...
> greenMpole(139,280): value= 0.000487023 term=-0.000000503 eps=0.001034
> greenMpole(140,282): value= 0.000487519 term= 0.000000496 eps=0.001018
> greenMpole(141,284): value= 0.000487030 term=-0.000000489 eps=0.001004
> greenMpole(142,286): value= 0.000487512 term= 0.000000482 eps=0.000989
> greenPwc: CollocationGreenPwc(val= 0.000487512)
31,34c171,371
< greenMpole( 2,  7): value= 0.000244699 term= 0.000000167 eps=0.000681
< greenPwc: CollocationGreenPwc(val= 0.000244699)
---
> greenMpole( 0,  2): value= 1.225283582 term= 1.225283582 eps=1.000000
> greenMpole( 1,  4): value=-0.002254421 term=-1.227538003 eps=544.502614
> greenMpole( 2,  6): value= 0.001077528 term= 0.003331948 eps=3.092216
> greenMpole( 3,  8): value=-0.000171717 term=-0.001249244 eps=7.275034 <----
> greenMpole( 4, 10): value= 0.000494415 term= 0.000666132 eps=1.347313
> greenMpole( 5, 12): value= 0.000078166 term=-0.000416250 eps=5.325219 <----
> greenMpole( 6, 14): value= 0.000363537 term= 0.000285371 eps=0.784986
> greenMpole( 7, 16): value= 0.000155495 term=-0.000208042 eps=1.337929 <----
> greenMpole( 8, 18): value= 0.000313972 term= 0.000158476 eps=0.504747
> greenMpole( 9, 20): value= 0.000189197 term=-0.000124775 eps=0.659500 <----
> greenMpole(10, 22): value= 0.000290005 term= 0.000100808 eps=0.347609
> greenMpole(11, 24): value= 0.000206855 term=-0.000083150 eps=0.401974 <----
> greenMpole(12, 26): value= 0.000276617 term= 0.000069762 eps=0.252199
> greenMpole(13, 28): value= 0.000217248 term=-0.000059369 eps=0.273279 <----
> greenMpole(14, 30): value= 0.000268386 term= 0.000051139 eps=0.190541
> greenMpole(15, 32): value= 0.000223877 term=-0.000044509 eps=0.198810 <----
> greenMpole(16, 34): value= 0.000262968 term= 0.000039090 eps=0.148651
> greenMpole(17, 36): value= 0.000228363 term=-0.000034604 eps=0.151532 <----
> greenMpole(18, 38): value= 0.000259212 term= 0.000030848 eps=0.119009
> greenMpole(19, 40): value= 0.000231539 term=-0.000027672 eps=0.119515 <----
> greenMpole(20, 42): value= 0.000256502 term= 0.000024963 eps=0.097319
> greenMpole(21, 44): value= 0.000233870 term=-0.000022632 eps=0.096772
  ...
> greenMpole(196,394): value= 0.000244771 term= 0.000000250 eps=0.001023
> greenMpole(197,396): value= 0.000244523 term=-0.000000248 eps=0.001013
> greenMpole(198,398): value= 0.000244768 term= 0.000000245 eps=0.001002
> greenMpole(199,400): value= 0.000244525 term=-0.000000243 eps=0.000992
> greenPwc: CollocationGreenPwc(val= 0.000244525)
36,39c373,570
< greenMpole( 2,  7): value= 0.000250555 term= 0.000000167 eps=0.000665
< greenPwc: CollocationGreenPwc(val= 0.000250555) val= 0.000247627
---
> greenMpole( 0,  2): value= 1.254569203 term= 1.254569203 eps=1.000000
> greenMpole( 1,  4): value=-0.002248563 term=-1.256817766 eps=558.942617
> greenMpole( 2,  6): value= 0.001083384 term= 0.003331947 eps=3.075499
> greenMpole( 3,  8): value=-0.000165860 term=-0.001249244 eps=7.531917 <----
> greenMpole( 4, 10): value= 0.000500272 term= 0.000666132 eps=1.331540
> greenMpole( 5, 12): value= 0.000084022 term=-0.000416250 eps=4.954039 <----
> greenMpole( 6, 14): value= 0.000369394 term= 0.000285371 eps=0.772540
> greenMpole( 7, 16): value= 0.000161352 term=-0.000208042 eps=1.289366 <----
> greenMpole( 8, 18): value= 0.000319828 term= 0.000158476 eps=0.495504
> greenMpole( 9, 20): value= 0.000195053 term=-0.000124775 eps=0.639698 <----
> greenMpole(10, 22): value= 0.000295861 term= 0.000100808 eps=0.340728
> greenMpole(11, 24): value= 0.000212711 term=-0.000083150 eps=0.390907 <----
> greenMpole(12, 26): value= 0.000282474 term= 0.000069762 eps=0.246970
> greenMpole(13, 28): value= 0.000223104 term=-0.000059369 eps=0.266105 <----
> greenMpole(14, 30): value= 0.000274243 term= 0.000051139 eps=0.186472
> greenMpole(15, 32): value= 0.000229734 term=-0.000044509 eps=0.193742 <----
> greenMpole(16, 34): value= 0.000268824 term= 0.000039090 eps=0.145412
> greenMpole(17, 36): value= 0.000234220 term=-0.000034604 eps=0.147743 <----
> greenMpole(18, 38): value= 0.000265068 term= 0.000030848 eps=0.116379
> greenMpole(19, 40): value= 0.000237396 term=-0.000027672 eps=0.116567 <----
> greenMpole(20, 42): value= 0.000262359 term= 0.000024963 eps=0.095147
> greenMpole(21, 44): value= 0.000239727 term=-0.000022632 eps=0.094408
> greenMpole(22, 46): value= 0.000260340 term= 0.000020613 eps=0.079177
> greenMpole(23, 48): value= 0.000241487 term=-0.000018852 eps=0.078068
  ...
> greenMpole(193,388): value= 0.000250374 term=-0.000000258 eps=0.001032
> greenMpole(194,390): value= 0.000250630 term= 0.000000256 eps=0.001020
> greenMpole(195,392): value= 0.000250377 term=-0.000000253 eps=0.001010
> greenMpole(196,394): value= 0.000250627 term= 0.000000250 eps=0.000999
> greenPwc: CollocationGreenPwc(val= 0.000250627) val= 0.000247576
42,44c573,708
< greenMpole( 1,  5): value= 0.000542670 term=-0.000000500 eps=0.000921
< greenPwc: CollocationGreenPwc(val= 0.000542670)
---
> greenMpole( 0,  2): value= 2.716121090 term= 2.716121090 eps=1.000000
> greenMpole( 1,  4): value=-0.001956303 term=-2.718077393 eps=1389.395058
> greenMpole( 2,  6): value= 0.001375667 term= 0.003331970 eps=2.422075
> greenMpole( 3,  8): value= 0.000126421 term=-0.001249247 eps=9.881674 <----
> greenMpole( 4, 10): value= 0.000792553 term= 0.000666133 eps=0.840489
> greenMpole( 5, 12): value= 0.000376303 term=-0.000416250 eps=1.106155 <----
> greenMpole( 6, 14): value= 0.000661675 term= 0.000285371 eps=0.431287
> greenMpole( 7, 16): value= 0.000453633 term=-0.000208042 eps=0.458613 <----
> greenMpole( 8, 18): value= 0.000612109 term= 0.000158476 eps=0.258902
> greenMpole( 9, 20): value= 0.000487334 term=-0.000124775 eps=0.256036
> greenMpole(10, 22): value= 0.000588142 term= 0.000100808 eps=0.171401
> greenMpole(11, 24): value= 0.000504992 term=-0.000083150 eps=0.164656
  ...
> greenMpole(131,264): value= 0.000542502 term=-0.000000568 eps=0.001046
> greenMpole(132,266): value= 0.000543061 term= 0.000000559 eps=0.001029
> greenMpole(133,268): value= 0.000542511 term=-0.000000551 eps=0.001015
> greenMpole(134,270): value= 0.000543053 term= 0.000000542 eps=0.000998
> greenPwc: CollocationGreenPwc(val= 0.000543053)
68c732
<     res 12.09198k (a, SUBSTR); /* new */
>     res 12.09543k (a, SUBSTR); /* old */

Note: I have set min_divergence_term=30 to forcome false divergence stops (see <---).
      By min_divergence_term=10, res=10.64889k.
      By maxGreenTerms =100,     res=12.07560k.
      By collocationEps=0.002,   res=12.09392k.
.C0
.SK
.HU "APPENDIX F -- Results: grounded substrate, nLayers=3 (2 spiders)"
.C8
prepassing single for substrate resistance
greenInit: use_lowest_medium=1 use_old_images=0
greenInit: use_mean_green_values=0 merge_images=1
greenInit: use_multipoles=1 test_multipoles=0 min_divergence_term=30
greenInit: be_mode='collocation' FeModeGalerkin=0 FeModePwl=0
greenInit: reading substrate specification
greenInit: i=1 m='epi' s=10 t=0
greenInit: i=2 m='epi2' s=40 t=-100
greenInit: i=3 m='metalization' s=1 t=-250
greenInit: sub3d.neumann_simulation_ratio=10000
greenInit: i=1 e=40 b=0
greenInit: i=2 e=10 b=150
greenInit: i=3 e=0.001 b=250
greenInit: greenType=3 nLayers=3 maxGreenTerms=500 collocationEps=0.001
strip 0 300 (add)
Schur dimension 2, maxorder 1
greenPwc: d=0 r=4800 sp1=(850,1200,100000) sp2=(850,1200,100000)
greenMpole: spo=(850,1200,100000) spc=(850,1200,100000) cnt=1 convRadius=1250 gs3322
greenMpole( 0,  2): value= 0.000243854 term= 0.000243854 eps=1.000000
greenMpole( 1,  9): value= 0.000486061 term= 0.000242207 eps=0.498306
greenMpole( 2, 22): value= 0.000485946 term=-0.000000115 eps=0.000237
greenPwc: CollocationGreenPwc(val= 0.000485946)
greenPwc: d=600 r=4800 sp1=(850,1200,100000) sp2=(250,1200,100000)
greenMpole: spo=(850,1200,100000) spc=(250,1200,100000) cnt=2 convRadius=1225.77 gs3322
greenMpole( 0,  2): value= 0.000122528 term= 0.000122528 eps=1.000000
greenMpole( 1,  9): value= 0.000243434 term= 0.000120906 eps=0.496668
greenMpole( 2, 22): value= 0.000243319 term=-0.000000115 eps=0.000473
greenPwc: CollocationGreenPwc(val= 0.000243319)
greenMpole: spo=(250,1200,100000) spc=(850,1200,100000) cnt=3 convRadius=1250 gs3322
greenMpole( 0,  2): value= 0.000125457 term= 0.000125457 eps=1.000000
greenMpole( 1,  9): value= 0.000249291 term= 0.000123834 eps=0.496745
greenMpole( 2, 22): value= 0.000249176 term=-0.000000115 eps=0.000462
greenPwc: CollocationGreenPwc(val= 0.000249176) val= 0.000246247
greenPwc: d=0 r=4800 sp1=(250,1200,100000) sp2=(250,1200,100000)
greenMpole: spo=(250,1200,100000) spc=(250,1200,100000) cnt=4 convRadius=1225.77 gs3322
greenMpole( 0,  2): value= 0.000271612 term= 0.000271612 eps=1.000000
greenMpole( 1,  9): value= 0.000541572 term= 0.000269960 eps=0.498475
greenMpole( 2, 22): value= 0.000541457 term=-0.000000115 eps=0.000213
greenPwc: CollocationGreenPwc(val= 0.000541457)
extracting single
extraction statistics for layout single:
        capacitances        : 0
        resistances         : 1
        nodes               : 2
        mos transistors     : 0
        bipolar vertical    : 0
        bipolar lateral     : 0
        substrate terminals : 1
        substrate nodes     : 1

space3d: --- Finished ---

/* Date: 12-Apr-02 15:51:34 GMT */
network single (terminal a)
{
    res 12.04912k (a, SUBSTR);
}
.C0
.SK
.HU "APPENDIX G -- Results: grounded substrate, nLayers=3 (1 spider)"
.C8
prepassing single for substrate resistance
greenInit: use_lowest_medium=1 use_old_images=0
greenInit: use_mean_green_values=0 merge_images=1
greenInit: use_multipoles=1 test_multipoles=0 min_divergence_term=3
greenInit: be_mode='collocation' FeModeGalerkin=0 FeModePwl=0
greenInit: reading substrate specification
greenInit: i=1 m='epi' s=10 t=0
greenInit: i=2 m='epi2' s=40 t=-100
greenInit: i=3 m='metalization' s=1 t=-250
greenInit: sub3d.neumann_simulation_ratio=10000
greenInit: i=1 e=40 b=0
greenInit: i=2 e=10 b=150
greenInit: i=3 e=0.001 b=250
greenInit: greenType=3 nLayers=3 maxGreenTerms=500 collocationEps=0.001
strip 0 300 (add)
Schur dimension 1, maxorder 0
greenPwc: d=0 r=4800 sp1=(600,1200,100000) sp2=(600,1200,100000)
greenMpole: spo=(600,1200,100000) spc=(600,1200,100000) cnt=1 convRadius=1341.64 gs3322
greenMpole( 0,  2): value= 0.000200005 term= 0.000200005 eps=1.000000
greenMpole( 1,  9): value= 0.000398372 term= 0.000198367 eps=0.497944
greenMpole( 2, 22): value= 0.000398257 term=-0.000000115 eps=0.000289
greenPwc: CollocationGreenPwc(val= 0.000398257)
extracting single
extraction statistics for layout single:
        capacitances        : 0
        resistances         : 1
        nodes               : 2
        mos transistors     : 0
        bipolar vertical    : 0
        bipolar lateral     : 0
        substrate terminals : 1
        substrate nodes     : 1

overall resource utilization:
        memory allocation  : 0.188 Mbyte
	user time          :         0.0
	system time        :         0.0
	real time          :         0.2  25%

space3d: --- Finished ---

/* Date: 12-Apr-02 16:07:18 GMT */
network single (terminal a)
{
    res 12.67691k (a, SUBSTR);
}

---------------------------------------------------------------------------------
t= -10 res=12.02694k (2, 22): value=0.000377837 eps=0.000648
t= -50 res=12.60892k (2, 22): value=0.000396121 eps=0.000273
t=-100 res=12.67691k (2, 22): value=0.000398257 eps=0.000289
t=-125 res=12.68890k (2, 13): value=0.000398634 eps=0.000719
t=-150 res=12.69902k (3, 38): value=0.000398951 eps=0.000255
t=-200 res=12.71611k (5, 88): value=0.000399488 eps=0.000828
t=-240 res=12.74526k (7,212): value=0.000400404 eps=0.003870 (divergence stop)
---------------------------------------------------------------------------------
.C0
