.subckt att_ref iref comp4 comp1 comp2 comp3 comp6 rshunt comp5 ireg vreg 
+               switch asssymm rschaal2 rschaal1 rcshunt rv ground outneg Vcc
+                outpos compv comcomp
q1 comp4 1 Vcc pnpWP
q2 comp2 1 rschaal1 pnpWP
q3 comp5 1 rschaal2 pnpWP
q4 1 1 Vcc pnpWP
q5 comp2 comp1 ground npnBW
q6 1 iref ground npnBW
q7 1 iref ground npnBW
q8 1 iref ground npnBW
q9 2 2 Vcc pnpWP
q10 comp5 comp4 ground npnBW
q11 comp1 1 Vcc pnpWP
q12 comp1 comp1 ground npnBW
q13 iref iref ground npnBW
q14 rshunt 2 Vcc pnpWP
q15 comp4 comp4 ground npnBW
q16 2 3 ground npnBW
q17 comp1 comp1 ground npnBW
q18 rcshunt rshunt Vcc pnpWP
q19 comp4 comp4 ground npnBW
q20 3 3 ground npnBW
q21 switch iref ground npnBW
q22 comp1 comp3 ground npnBW
q23 rcshunt iref ground npnBW
q24 rcshunt iref ground npnBW
q25 rcshunt iref ground npnBW
q26 comp4 comp6 ground npnBW
q27 comp6 rcshunt Vcc pnpWP
q28 comp3 comp2 ground npnBW
q29 3 4 Vcc pnpWP
q30 rshunt iref ground npnBW
q31 compv switch Vcc pnpWP
q32 compv switch Vcc pnpWP
q33 comp6 comp5 ground npnBW
q34 4 comcomp ground npnBW
q35 comp3 rcshunt Vcc pnpWP
q36 8 8 Vcc pnpWP
q37 ireg 8 Vcc pnpWP
q38 outneg comp2 vreg npnBW
q39 comcomp comp2 vreg npnBW
q40 comcomp comp5 vreg npnBW
q41 outpos comp5 vreg npnBW
q42 5 switch Vcc pnpWP
q43 5 switch Vcc pnpWP
q44 5 5 ireg npnBW
q45 compv 5 rv npnBW
q46 6 compv Vcc pnpWP
q47 Vcc 6 rv npnBW
q48 switch switch Vcc pnpWP
q49 switch switch Vcc pnpWP
q50 switch switch Vcc pnpWP
q51 switch switch Vcc pnpWP
q52 switch switch Vcc pnpWP
q53 switch switch Vcc pnpWP
q54 switch switch Vcc pnpWP
q55 switch switch Vcc pnpWP
q56 switch switch Vcc pnpWP
q57 switch switch Vcc pnpWP
q58 switch switch Vcc pnpWP
q59 switch switch Vcc pnpWP
q60 switch switch Vcc pnpWP
q61 switch switch Vcc pnpWP
q62 switch switch Vcc pnpWP
q63 switch switch Vcc pnpWP
q64 switch switch Vcc pnpWP
q65 switch switch Vcc pnpWP
q66 switch switch Vcc pnpWP
q67 switch switch Vcc pnpWP
q68 switch switch Vcc pnpWP
q69 switch switch Vcc pnpWP
q70 switch switch Vcc pnpWP
q71 switch switch Vcc pnpWP
q72 switch switch Vcc pnpWP
q73 switch switch Vcc pnpWP
q74 switch switch Vcc pnpWP
q75 switch switch Vcc pnpWP
q76 switch switch Vcc pnpWP
q77 switch switch Vcc pnpWP
q78 switch switch Vcc pnpWP
q79 switch switch Vcc pnpWP
q80 switch switch Vcc pnpWP
q81 7 switch Vcc pnpWP
q82 7 switch Vcc pnpWP
q83 7 7 ground npnBW
q84 7 7 ground npnBW
q85 7 7 ground npnBW
q86 6 9 ground npnBW
q87 9 9 ground npnBW
q88 8 7 ground npnBW
q89 8 7 ground npnBW
q90 8 7 ground npnBW
q91 8 7 ground npnBW
q92 8 7 ground npnBW
q93 comcomp switch Vcc pnpWP
q94 comcomp switch Vcc pnpWP
q95 9 switch Vcc pnpWP
q96 9 switch Vcc pnpWP
q97 outneg asssymm Vcc pnpWP
q98 outpos asssymm Vcc pnpWP
* end att_ref
.ends