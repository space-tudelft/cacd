.T= "Test Results of the New Green Functions"
.DS 2
.rs
.sp 1i
.B
.S 15 20
TEST RESULTS
OF THE NEW GREEN FUNCTIONS
OF THE SPACE PROGRAM
.S
.sp 1
.I
S. de Graaf
.sp 1
.R
Circuits and Systems Group
Faculty of Electrical Engineering,
Mathematics and Computer Science
Delft University of Technology
The Netherlands
.DE
.sp 2c
.ce
Report EWI-ENS 04-09
.ce
September 2, 2004
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2004 by the author.

Last revision: September 9, 2004.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This document contains the first test results of the new Green function implementation.
The new Green functions are added by Kees-Jan van der Kolk to the
.P= space3d
program.
More than 3 dielectrical layers can be used with the new Green interface.
.P
The new Green interface is only used by
.P= space3d ,
when the special technology Green files exist.
A new version of the technology compiler
.P= tecc
must be used with option
.B -u
to generate these special Green files.
This is a time consuming job, because the
.P= tecc
program calculates a big number of Green values for a lot of situations.
.br
The following picture shows how it works.
.P
.F+
.PSPIC "an0409/fig1.ps" 5i 3i
.F-
.P
To use the old getImageGroup method, set parameter "cap3d.use_unigreen" to "off".
Note that the old method is used by
.P= space3d ,
when there are no special ``unigreen'' technology files.
.br
Note that the
.P= tecc
program does no special compilations for the "sublayers".
.H 1 "MY FIRST TEST"
I started my first cap3d extraction testcase with the layout of cell "coilgen".
But looking to the technology file "c65/space.def.s", i discovered, that
the values for the dielectric heights were out of proportion.
Because 1900 micron for SiO2Top and 3900 micron for passivation
looks me too high.
Note that both dielectrics can be left out and that gives the same results.
Maybe the user was thinking in 1e-9 vdimension units.
I have rewritten all values in microns.
See appendix A for the used test values.
The "coilgen" layout contains only interconnect layers m5 and m6.
Note that the height of 1.9 mu for SiO2Top crosses the m5 layer.
Thus i started my first test with 3 dielectric interfaces and with maybe a wrong dielectric height position.
The vdimension of layer m7 is commented out, because is was laying too high for the new
.P= tecc
method.
See appendix B1 for these technology compilation results.
.P
The appendix B1 shows, that the old
.P= tecc
and new
.P= tecc
without option
.B -u
gives the same results.
See also, what the first time happened, when using option
.B -u .
The error message is very long, but is folded in the appendix.
Thus, in the new method (when using option
.B -u ),
the highest dielectric interface position must be \(>= the highest vdimension (incl. thickness).
In the new
.P= tecc
program, the
.B -v
option is used to view the progress.
This gives, however, a lot of output.
At last, appendix B1 shows how many computation time is used.
.P
Note that the old
.P= tecc
program*
.FS *
The new
.P= tecc
program stops, like the old program, when there are more than 6 sublayers.
.FE
stops compiling, when there are more than 6 dielectric interfaces.
The new program gives only a warning message, when there are more than 3 dielectric interfaces
without using option
.B -u .
.H 1 "TEST OF poly5"
We can take the "poly5" example from the "Space 3D Capacitance Extraction" manual, to be sure
that everything is working fine.
Appendix D shows that the capacitance values for the old and new method are almost the same.
The following table shows the
.P= space3d
execution times in seconds.
.fS
    |==== 2 dielectrics ====|==== 3 dielectrics ====|
    |m=0.5|m=0.5|m=0.1|m=0.1|m=0.5|m=0.5|m=0.1|m=0.1| m = max_be_area
    |old  |new  |old  |new  |old  |new  |old  |new  |
| w |time |time |time |time |time |time |time |time | w = be_window (1e-6 m)
+---+-----+-----+-----+-----+-----+-----+-----+-----+
| 1 | 0.03| 0.06| 0.37| 0.74| 0.07| 0.11| 1.18| 1.42|
| 2 | 0.08| 0.18| 1.34| 1.80| 0.33| 0.32| 4.98| 4.92|
| 3 | 0.13| 0.26| 2.96| 4.60| 0.66| 0.56| 9.26| 8.66|
| 4 | 0.15| 0.27| 3.06| 5.32| 0.79| 0.60|13.65| 9.82|
| 5 | 0.17| 0.28| 4.11| 5.94| 0.86| 0.62|15.36|10.13|
+---+-----+-----+-----+-----+-----+-----+-----+-----+
.fE
The results show, that by 2 dielectrics the old version is faster,
but by 3 dielectrics the new version becomes faster.
Note, how larger the be_window, how greater the difference.
.nr Ej 0
.H 1 "TEST OF sram"
.nr Ej 1
We can also take the "sram" example from the "Space 3D Capacitance Extraction" manual, to be sure
that everything is working fine.
.P
Appendix E shows the execution time results.
Note that the old
.P= space3d
program executes faster for 3 dielectrics than 2 dielectrics,
when there is used a dummy 2nd SiO2 dielectric with same epsilon.
There was a
.P= tecc
compilation problem for example J.
But this is already fixed.
.P
The results show, that the new
.P= space3d
program executes faster for 3 dielectrics, when these 3 dielectrics have different epsilons
(see examples C, D, E and H).
But, the results show also, that the
.P= tecc
compilation time is also long for these examples.
.H 1 "APPENDICES"
.SP
.nr Hu 2
.HU "APPENDIX A -- Test of cell: coilgen"
.nf
.S 8
.ft C
==== c65/space.def.p ====

cap3d.be_window     10
cap3d.max_be_area   10
cap3d.edge_be_ratio  1    ## default value

==== c65/space.def.s ====

unit vdimension   1e-6

vdimensions:        ##    h    thickness
        dim1 : m1 : m1 : 0.470 0.135
        dim2 : m2 : m2 : 0.765 0.175
        dim3 : m3 : m3 : 1.100 0.175
        dim4 : m4 : m4 : 1.435 0.175
        dim5 : m5 : m5 : 1.890 0.350
        dim6 : m6 : m6 : 2.900 0.570
    ##  dim7 : m7 : m7 : 3.900 1.300

dielectrics:        ## epsilon   h (mu)
        SiO2            2.65    0.0
        SiO2Top         3.75    1.9
        passivation     5.3     3.9



#--------------- 5.200
###### m7 ###### 1.300
#--------------- 3.900 <- - - 3.9 passivation
#
#--------------- 3.470
###### m6 ###### 0.570
#--------------- 2.900
#                      <- - - 2.5
#--------------- 2.240
###### m5 ###### 0.350 <- - - 1.9  SiO2Top
#--------------- 1.890
#                      <- - - 1.7
#--------------- 1.610
###### m4 ###### 0.175
#--------------- 1.435
#                            SiO2
#--------------- 0.0 - <- - - 0.0
#////////////////////////////////
#////////////////////////////////
.ft
.S
.SK
.HU "APPENDIX B1 -- Technology compilation results"
.nf
.S 8
.ft C
==== c65/space.def.s =============
dielectrics:        ## epsilon   h
        SiO2            2.65    0.0
        SiO2Top         3.75    1.9
        passivation     5.3     3.9

=======================================================================
c65 % tecc -m maskdata space.def.s
c65 % ~/unigreen/cacd/bin/tecc -m maskdata space.def.s

-- keys: m1 m2 m3 m4 m5 m6 m7 diff cont v1 v2 v3
-- keys2:
-- number of keys: 0 + 12 (15)
-- number of keys2: 0 + 0 (0)
-- number of key slots: 4096 (1)
-- maximum number of elements per key slot: 22 (0)
-- maximum number of additional conditions per element: 1
-- average number of additional conditions per element: 0.086

-- add. cond.  :  0  1
   no. of elem.: 32  3 (35)

=======================================================================
c65 % ~/unigreen/cacd/bin/tecc -u -m maskdata space.def.s
   ...
error: The highest point reachable by a conductor should be below the highest
dielectric interface. Please verify your `vdimensions' and `dielectrics' section.

=======================================================================
c65 % vi space.def.s      ## removing m7 from vdimensions
c65 % time ~/unigreen/cacd/bin/tecc -u -m maskdata space.def.s
   ...
message: Computation may take a long time. Use `-v' to view progress.

685.110u 1.290s 11:29.04 99.6%  0+0k 0+0io 1347pf+0w (SiO2Top h=1.9)
588.980u 2.500s  9:56.24 99.2%  0+0k 0+0io 1348pf+0w (SiO2Top h=1.7)
824.410u 2.760s 13:50.02 99.6%  0+0k 0+0io 1348pf+0w (SiO2Top h=2.5)

=======================================================================
Other timing results using "tecc -u space.def.s":

+------------+--------+--------------------------------------------
|            | user   | diel interface
|            | time(s)| values:
+------------+--------+--------------------------------------------
|1 real diel |   0.02 | 2.65 0 |
|1 real diel | 163.27 | 2.65 0 | 2.65 10           simulated with 2
|1 real diels| 192.59 | 2.65 0 | 2.65 10 | 2.65 20 simulated with 3
|2 real diels| 338.42 | 2.65 0 | 3.75 1.9 |
|3 real diels| 860.54 | 2.65 0 | 3.75 1.9 | 5.3 3.9 |
|3 real diels| 859.25 | 2.65 0 | 3.75 1900 | 5.3 3900 |
+------------+--------+--------------------------------------------
.ft
.S
.SK
.HU "APPENDIX B2 -- Technology compilation results"
.nf
.S 8
.ft C
using "tecc -u space.def.s" with epsilon 2.65:
=======================================================================
| file size (bytes)  | user  |nr of| diel
| dielc    | dielc#l | time  |diels| positions
+----------+---------+-------+-----+---------------------------------
|          |         |   0.0 |   1 | 0
|          |         | 207.7 |   3 | 0 0.1  3.47
|          |         | 208.5 |   3 | 0 0.01 3.47
|          |         | 207.1 |   3 | 0 0.1  3.50
|          |         | 203.4 |   3 | 0 0.5  3.50
+----------+---------+-------+-----+---------------------------------
|14.289.948|7.800.184| 162.6 |   2 | 0 3.47
|14.290.694|7.801.336| 203.6 |   3 | 0 3.47 3.48
|14.291.666|7.803.224| 257.5 |   4 | 0 3.47 3.48 3.49
|14.293.251|7.805.848| 328.2 |   5 | 0 3.47 3.48 3.49 3.50
|13.947.376|7.617.892| 403.4 |   6 | 0 3.47 3.48 3.49 3.50 3.51
|13.949.096|7.621.988| 501.5 |   7 | 0 3.47 3.48 3.49 3.50 3.51 3.52
|13.953.746|7.626.820| 611.6 |   8 | 0 3.47 3.48 3.49 3.50 3.51 3.52 ...
|13.958.323|7.632.388| 736.8 |   9 | 0 3.47 3.48 3.49 3.50 3.51 3.52 ...
|13.618.908|7.449.752| 856.1 |  10 | 0 3.47 3.48 3.49 3.50 3.51 3.52 ...
+----------+---------+-------+-----+---------------------------------
|          |         |   859 |   3 | 2.65 0/     3.75 1900/5.3 3900
|14.626.662|8.043.572|  1557 |   4 | .../2.7 1.7/...
|15.026.374|8.211.916|  4372 |   5 | .../2.7 1.7/2.8 2.5/...
|15.469.735|8.419.172|  7157 |   6 | .../2.7 1.7/2.8 2.5/3.65 3.5/...
|18.551.122|9.713.012| 24657 |   7 | .../2.7 1.7/2.75 2.3/2.8 2.5/3.65 3.5/...
+----------+---------+-------+-----+---------------------------------
|14.643.409|8.011.596|   907 |   3 | 2.65 0/     3.75 2.9/5.3 3.9
|14.745.107|8.055.820|  1614 |   4 | .../2.7 1.7/...
|15.451.825|8.396.944|  3371 |   5 | .../2.7 1.7/2.8 2.5/...
|16.020.340|8.641.872|  5767 |   6 | .../2.7 1.7/2.8 2.5/.../5.0 3.5/...
|17.931.030|9.438.960| 13600 |   7 | .../2.7 1.7/2.75 2.3/2.8 2.5/...
|20.372.475|10556.804| 20350 |   8 | .../2.7 1.7/2.75 2.3/2.8 2.5/2.85 2.7/...
+----------+---------+-------+-----+---------------------------------

.ft
.S
.HU "APPENDIX B3 -- Some technology compilation problems"
.nf
.S 8
.ft C
=======================================================================
dielectrics:        ## epsilon   h
        SiO2            2.65    0.0
        SiO2x           2.65    3.47
        SiO2x           2.65    3.47

tecc: error: Multiple use of element name SiO2x

=======================================================================
dielectrics:        ## epsilon   h
        SiO2            2.65    0.0
        SiO2x           2.65    3.47
        SiO2y           2.65    3.47

message: Computation may take a long time. Use `-v' to view progress.
terminate called after throwing a `libstd::STDException'
  what(): error: Unable to compute green's function because zp is above top layer.
/users/simon/unigreen/cacd/bin/tecc: line 100: 10432 Aborted
                                  (core dumped) $dist_bin/../../$arch/bin/$tail $*
.ft
.S
.SK
.HU "APPENDIX C1 -- space3d test results for cell coilgen"
.nf
.S 8
.ft C
================================================================================
TEST with 3 real dielectric interfaces (with different 2nd position)
================================================================================
% time ~/unigreen/cacd/bin/space3d -3C coilgen -Suse_multipoles=off -Scap3d.use_unigreen=on
330.160u 0.490s 5:32.11 99.5%uti0+0k 0+0io 1393pf+0w

% xsls coilgen

network coilgen (terminal NL1, port1, port2, NR1, SL1, SE1, SW1, NW1, WU1, NW2,
                 SR1, SW2, EL2, SE2, EU1, WL2, WU2, NE2, NL2, NR2, WL1, EU2,
                 SL2, SR2, NE1, EL1)
{
    ...
    net {SW1, SW2};
    cap 94.69634f (SW1, GND);
}

(1) dielectrics: (SiO2 2.65 0 | SiO2Top 3.75 1.7 | passivation 5.3 3.9)
(2) dielectrics: (SiO2 2.65 0 | SiO2Top 3.75 1.9 | passivation 5.3 3.9)
(3) dielectrics: (SiO2 2.65 0 | SiO2Top 3.75 2.5 | passivation 5.3 3.9)

 TEST RESULTS:
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
|space3d|use_      |use_    | (1)     (2)     (3)   | (1) cap | (2) cap | (3) cap |
|version|multipoles|unigreen|time(s)|time(s)|time(s)|value(fF)|value(fF)|value(fF)|
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
|  old  |  off     |  off   | 527.6 | 616.4 | 454.5 | 96.7709 | 94.7264 | 94.2240 |
|  new  |  off     |  off   | ===== | ===== | ===== | ======= | ======= | ======= |
|  new  |  off     |  on    | 259.7 | 330.2 | 273.0 | 96.7369 | 94.6963 | 94.1950 |
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
|  old  |  on      |  off   |  32.0 |  38.8 |  28.4 | 96.8703 | 94.8045 | 92.9775 |
|  new  |  on      |  off   |  34.1 |  60.2 |  28.6 | 96.8703 | 94.8045 | 92.9775 |
|  new  |  on      |  on    |  11.4 |  14.3 |  13.0 | 96.8585 | 94.8053 | 92.9779 |
+-------+----------+--------+-------+-------+-------+---------+---------+---------+


 TEST RESULTS: space3d=old, use_multipoles=on
+----------------+-------+---------+-wrong-mp+
|SiO2Top position|time(s)|value(fF)|value(fF)|
+----------------+-------+---------+---------+
| 1)       1.7   |  32.0 | 96.8703 | 96.8703 |
|          1.9   |  38.8 | 94.8045 | 91.6837 |
|          2.5   |  28.4 | 92.9775 | 89.7539 |
|          2.9   |  27.6 | 92.4087 | 89.6380 |
|          3.2   |  32.7 | 92.0606 | 89.9470 |
| 1)       3.5   |  24.1 | 91.4462 | 91.4462 |
+----------------+-------+---------+---------+
1) No diff, because spiders m5/m6 in same dielectric.

#                      <- - - 3.5
#--------------- 3.470
###### m6 ###### 0.570 <- - - 3.2
#--------------- 2.900 <- - - 2.9
#                      <- - - 2.5
#--------------- 2.240
###### m5 ###### 0.350 <- - - 1.9  SiO2Top
#--------------- 1.890
#                      <- - - 1.7
.ft
.S
.SK
.HU "APPENDIX C2 -- space3d test results for cell coilgen"
.nf
.S 8
.ft C
================================================================================
TEST with 1 real dielectric interface
================================================================================
(1) dielectrics: (SiO2 2.65 0)
(2) simulated 2: (SiO2 2.65 0 | SiO2x 2.65 10)
(3) simulated 3: (SiO2 2.65 0 | SiO2x 2.65 10 | SiO2y 2.65 20)

 TEST RESULTS:
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
|space3d|use_      |use_    | (1)     (2)     (3)   | (1) cap | (2) cap | (3) cap |
|version|multipoles|unigreen|time(s)|time(s)|time(s)|value(fF)|value(fF)|value(fF)|
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
|  old  |  off     |  off   |   7.5 |# 72.7 |>528.1 | 80.1936 | 80.1936 | *) core |
|  new  |  off     |  off   |   7.9 |# 79.0 |  ,,   | 80.1936 | 80.1936 | *) core |
|  new  |  off     |  on    |@  8.0 | 282.5 | 279.5 | 80.1936 | 80.1673 | 80.1665 |
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
|  old  |  on      |  off   |   2.0 |   2.8 |   3.4 | 80.2745 | 80.2745 | 80.2745 |
|  new  |  on      |  off   |   2.2 |   2.9 |   3.1 | 80.2745 | 80.2745 | 80.2745 |
|  new  |  on      |  on    |@  2.2 |   3.6 |   3.3 | 80.2745 | 80.2745 | 80.2745 |
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
@) message: Turning off unigreen method for dielectric case (no blob found).
#) space3d: Computation of Greens function truncated after 500 green_terms,
      error specified by green_eps not reached (layers are SiO2 and SiO2).
   space3d: Warning: maximum error not reached for 0.9% of the Greens functions.
*) space3d: No more core.
   Already allocated 232162959 bytes, cannot get 18934560 more.


.ft
.S
.HU "APPENDIX C3 -- space3d test results for cell coilgen"
.nf
.S 8
.ft C
================================================================================
TEST with 2/3 real dielectric interfaces at wrong position
================================================================================
(1) dielectrics: (SiO2 2.65 0)
(2) dielectrics: (SiO2 2.65 0 | SiO2Top 3.75 1900)
(3) dielectrics: (SiO2 2.65 0 | SiO2Top 3.75 1900 | passivation 5.3 3900)

 TEST RESULTS:  (see also appendix B2)
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
|space3d|use_      |use_    | (1)     (2)     (3)   | (1) cap | (2) cap | (3) cap |
|version|multipoles|unigreen|time(s)|time(s)|time(s)|value(fF)|value(fF)|value(fF)|
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
|  old  |  off     |  off   |   7.5 |  15.5 |  25.8 | 80.1936 | 80.1936 | 80.1936 |
|  new  |  off     |  off   |   7.9 |  17.1 |  28.0 | 80.1936 | 80.1936 | 80.1936 |
|  new  |  off     |  on    |   8.0 | 243.1 |* 22.9 | 80.1936 |171.9055 | *) NAN  |
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
|  old  |  on      |  off   |   2.0 |   2.9 |   3.3 | 80.2745 | 80.2745 | 80.2745 |
|  new  |  on      |  off   |   2.2 |   2.9 |   3.5 | 80.2745 | 80.2745 | 80.2745 |
|  new  |  on      |  on    |   2.2 |   6.5 |  15.9 | 80.2745 | 80.2745 | 80.2745 |
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
|  new  |  on      |  on    |  13.8 |==(3)==|  15.9 | 92.4132 | ==(3)== | 80.2745 |
|  new  |  on      |  on    |  25.1 |==(4)==|  24.5 | 92.6582 | ==(4)== | 80.7966 |
|  new  |  on      |  on    |  48.8 |==(5)==| 123.7 | 92.7174 | ==(5)== |#83.4778 |
|  new  |  on      |  on    |  66.0 |==(6)==| 304.7 | 93.4153 | ==(6)== |#93.7899 |
|  new  |  on      |  on    | 121.2 |==(7)==| 415.  | 93.4311 | ==(7)== |#93.8692 |
|  new  |  on      |  on    | 143.4 |==(8)==| ===== | 93.4424 | ==(8)== |#======= |
+-------+----------+--------+-------+-------+-------+---------+---------+---------+
*) space3d: Encountered NAN in schur module.5
#) warning: Using inaccurate set of Green's images.
1) in this column (2nd table part) the results of passivation 5.3 3.9
.ft
.S
.SK
.HU "APPENDIX D -- space3d test results for cell poly5"
.nf
.S 8
.ft C
dielectrics: SiO2  3.9 0.0
          ## SiO2b 3.0 2.0
             air   1.0 5.0

|OLD: max_be_area=0.5 be_mode=0c (2 dielectrics)
|w|Ca_b |Ca_G |Cb_G |Cc_G |Cd_G |Ce_G |Cb_c |Cc_d |Cd_e |Ca_c |Ca_d |Cb_d |Cc_e |Ca_e |Cb_e |
+-+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
|1|253.3|624.2|458.0|458.0|458.0|624.2|253.3|253.3|253.3|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|
|2|256.0|599.8|451.5|439.1|444.5|605.0|251.1|250.2|256.2|16.40| 7.93|16.40|18.79|xxxxx|xxxxx|
|3|256.8|593.3|444.4|435.2|452.0|593.4|251.3|259.2|256.9|16.69| 7.14|15.10|16.79| 4.49| 6.95|
|4|257.3|590.8|442.6|436.4|442.6|590.8|251.9|251.9|257.3|17.16| 7.22|15.02|17.16| 4.74| 7.22|
|5|257.4|590.4|442.3|436.1|442.3|590.4|251.9|251.9|257.4|17.22| 7.27|15.06|17.22| 4.78| 7.27|

|NEW: max_be_area=0.5 be_mode=0c (2 dielectrics)
|w|Ca_b |Ca_G |Cb_G |Cc_G |Cd_G |Ce_G |Cb_c |Cc_d |Cd_e |Ca_c |Ca_d |Cb_d |Cc_e |Ca_e |Cb_e |
+-+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
|1|253.3|624.2|457.9|457.9|457.9|624.2|253.3|253.3|253.3|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|
|2|256.0|599.8|451.5|439.1|444.5|605.0|251.1|250.2|256.2|16.38| 7.93|16.38|18.78|xxxxx|xxxxx|
|3|256.8|593.3|444.4|435.2|452.0|593.4|251.3|259.2|256.9|16.68| 7.14|15.09|16.78| 4.50| 6.95|
|4|257.4|590.8|442.6|436.4|442.6|590.8|251.9|251.9|257.4|17.15| 7.22|15.01|17.15| 4.74| 7.22|
|5|257.4|590.4|442.3|436.1|442.3|590.4|251.9|251.9|257.4|17.21| 7.27|15.05|17.21| 4.78| 7.27|

|OLD: max_be_area=0.1 be_mode=0c (2 dielectrics)
|w|Ca_b |Ca_G |Cb_G |Cc_G |Cd_G |Ce_G |Cb_c |Cc_d |Cd_e |Ca_c |Ca_d |Cb_d |Cc_e |Ca_e |Cb_e |
+-+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
|1|269.3|649.4|478.0|478.0|478.0|649.4|269.3|269.3|269.3|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|
|2|267.1|615.3|462.8|449.8|455.7|620.6|261.6|260.7|267.3|17.46| 8.09|17.46|19.96|xxxxx|xxxxx|
|3|268.1|607.4|454.7|446.4|458.9|607.3|262.1|266.3|268.5|17.95| 7.26|16.03|17.81| 4.64| 7.16|
|4|268.5|605.7|453.5|446.8|453.5|605.7|262.5|262.5|268.5|18.26| 7.35|16.02|18.26| 4.82| 7.35|
|5|268.6|605.2|453.1|446.4|453.1|605.2|262.5|262.5|268.6|18.33| 7.41|16.07|18.33| 4.88| 7.41|

|NEW: max_be_area=0.1 be_mode=0c (2 dielectrics)
|w|Ca_b |Ca_G |Cb_G |Cc_G |Cd_G |Ce_G |Cb_c |Cc_d |Cd_e |Ca_c |Ca_d |Cb_d |Cc_e |Ca_e |Cb_e |
+-+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
|1|269.3|649.4|478.0|478.0|478.0|649.4|269.3|269.3|269.3|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|
|2|267.1|615.3|462.8|449.8|455.7|620.6|261.7|260.7|267.3|17.45| 8.10|17.45|19.95|xxxxx|xxxxx|
|3|268.1|607.4|454.6|446.4|458.8|607.3|262.1|266.3|268.6|17.94| 7.26|16.02|17.80| 4.65| 7.16|
|4|268.5|605.7|453.5|446.8|453.5|605.7|262.5|262.5|268.5|18.25| 7.36|16.01|18.25| 4.82| 7.36|
|5|268.6|605.2|453.1|446.4|453.1|605.2|262.5|262.5|268.6|18.32| 7.41|16.06|18.32| 4.88| 7.41|

|OLD: max_be_area=0.1 be_mode=0c (3 dielectrics)
|w|Ca_b |Ca_G |Cb_G |Cc_G |Cd_G |Ce_G |Cb_c |Cc_d |Cd_e |Ca_c |Ca_d |Cb_d |Cc_e |Ca_e |Cb_e |
+-+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
|1|271.6|642.7|470.8|470.8|470.8|642.7|271.6|271.6|271.6|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|
|2|269.1|608.6|455.8|442.9|449.1|613.7|263.4|262.5|269.3|17.71| 7.70|17.71|20.14|xxxxx|xxxxx|
|3|270.1|601.4|448.2|439.7|452.3|601.3|263.9|268.0|270.5|18.14| 6.91|16.27|17.99| 4.14| 6.81|
|4|270.4|599.9|447.1|440.2|447.1|599.9|264.2|264.2|270.4|18.40| 6.98|16.23|18.40| 4.28| 6.98|
|5|270.5|599.5|446.9|439.9|446.9|599.5|264.3|264.3|270.5|18.47| 7.02|16.27|18.47| 4.33| 7.02|

|NEW: max_be_area=0.1 be_mode=0c (3 dielectrics)
|w|Ca_b |Ca_G |Cb_G |Cc_G |Cd_G |Ce_G |Cb_c |Cc_d |Cd_e |Ca_c |Ca_d |Cb_d |Cc_e |Ca_e |Cb_e |
+-+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
|1|271.5|642.7|470.9|470.9|470.9|642.7|271.5|271.5|271.5|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|xxxxx|
|2|269.1|608.7|455.8|442.9|449.1|613.7|263.4|262.5|269.3|17.72| 7.70|17.72|20.15|xxxxx|xxxxx|
|3|270.1|601.4|448.2|439.6|452.3|601.3|263.9|268.0|270.5|18.15| 6.91|16.28|18.00| 4.14| 6.81|
|4|270.4|599.7|447.1|440.2|447.1|599.9|264.2|264.2|270.4|18.41| 6.98|16.24|18.41| 4.28| 6.98|
|5|270.5|599.5|446.9|439.9|446.9|599.5|264.2|264.2|270.5|18.48| 7.02|16.28|18.48| 4.32| 7.02|

w = be_window (1e-6 m)
capacitance values (1e-18 F)
.ft
.S
.SK
.HU "APPENDIX E -- space3d test results for cell sram"
.nf
.S 8
.ft C
==== technology file "sram.s" (5 vdimensions) ====

---------------------------------------------------------
| A | 2 dielectrics: SiO2 3.9 0, air 1.0 5
---------------------------------------------------------
| B | 3 dielectrics: SiO2 3.9 0, SiO2b 3.9 4.0, air 1.0 5
| C | 3 dielectrics: SiO2 3.9 0, SiO2b 3.8 4.0, air 1.0 5
| D | 3 dielectrics: SiO2 3.9 0, SiO2b 3.0 4.0, air 1.0 5
| E | 3 dielectrics: SiO2 3.9 0, SiO2b 2.0 4.0, air 1.0 5
| F | 3 dielectrics: SiO2 3.9 0, SiO2b 1.0 4.0, air 1.0 5
---------------------------------------------------------
| G | 3 dielectrics: SiO2 3.9 0, SiO2b 3.9 2.6, air 1.0 5
| H | 3 dielectrics: SiO2 3.9 0, SiO2b 3.8 2.6, air 1.0 5
---------------------------------------------------------
| I | 3 dielectrics: SiO2 3.9 0, SiO2b 3.9 1.4, air 1.0 5
| J | 3 dielectrics: SiO2 3.9 0, SiO2b 3.8 1.4, air 1.0 5

==== timing results ====

    | technology file    | tecc    | space3d | space3d |
    |   "sram.t.dielc"   | new     | old     | new     |
    | size     | size #l | time(s) | time(s) | time(s) |
+---+----------+---------+---------+---------+---------+
| A | 14513308 | 7801512 | 295.96  |    1.55 |    2.48 |
+---+----------+---------+---------+---------+---------+
| B | 14523395 | 7806624 | 360.22  |    1.20 |    2.50 |
| C | 14441044 | 7813136 | 676.32  |    5.50 |    4.76 |
| D | 14426344 | 7814608 | 741.51  |    7.08 |    5.18 |
| E | 14536437 | 7813040 | 605.85  |    8.89 |    4.65 |
| F | 14407992 | 7805840 | 373.31  |    1.21 |    2.68 |
+---+----------+---------+---------+---------+---------+
| G | 14882498 | 8000316 | 361.35  |    1.26 |    2.21 |
| H | 14906466 | 8010188 | 694.25  |    5.75 |    5.18 |
+---+----------+---------+---------+---------+---------+
| I | 14883871 | 8000316 | 366.93  |    1.45 |    2.75 |
| J | ======== | ======= | ======  |    ==== |    ==== |

==== tecc compilation problem output (J) ====

% ~/unigreen/cacd/bin/tecc -u sram.s
-- keys: cpg caa cwn csn cmf cms cca ccp cva
-- keys2: cpg caa cwn csn cmf cms
-- number of keys: 6 + 3 (9)
-- number of keys2: 6 + 0 (6)
-- number of key slots: 512 (64)
-- maximum number of elements per key slot: 9 (2)
-- maximum number of additional conditions per element: 2
-- average number of additional conditions per element: 0.261

-- add. cond.  :  0  1  2
   no. of elem.: 19  2  2 (23)

message: Computation may take a long time. Use `-v' to view progress.

internal: *** Assertion `!(kplist[i] > previous)' failed at
          /u/52/52/work/keesjan/CACD/src/space/green/libunigreen/misc.h:238
/users/simon/unigreen/cacd/bin/tecc: line 100: 19608 Aborted (core dumped)
          $dist_bin/../../$arch/bin/$tail $*
.ft
.S
