.T= "Greens Functions Draft Incorrect?"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE APPLICATION NOTE
WHY I THINK THAT
GREEN'S FUNCTIONS FOR
CAPACITANCE CALCULATIONS
DRAFT ARE INCORRECT
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
Report ET-CAS 02-01-4
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
.H 1 "EXPLANATION"
See page 2-3 of the Draft the formules for the image strengths Si and positions Zi
below Equation (2.10), which must be equal to the \*(sP greenSeries(0,3,1,1):
.nf
.P
.L{
images.h:
---------
.sp .5
greenSeries (0, 3, 1, 1) {
    n = p(0) - p(2);
    u = p(0) + p(2);
    m = p(1) + p(2);
    S = k(1,u+1)*k(2,m)/e(1);
    image(-S,-2*t(1)*(n+1) - 2*t(2)*(m),-1); /*1*/
    image(-S, 2*t(1)*(n+1) + 2*t(2)*(m),-1); /*2*/
    image( S,-2*t(1)*(n+1) - 2*t(2)*(m), 1); /*3*/
    image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1); /*4*/
    S = k(1,u)*k(2,m+1)/e(1);
    image(-S,-2*t(1)*(n) - 2*t(2)*(m+1),-1); /*5*/
    image(-S, 2*t(1)*(n) + 2*t(2)*(m+1),-1); /*6*/
    image( S,-2*t(1)*(n) - 2*t(2)*(m+1), 1); /*7*/
    image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1); /*8*/
}
.sp .5
zq_sign=-1: R.z = Zp - Zq + distance; Zi = +Zq - distance;
zq_sign=+1: R.z = Zp + Zq + distance; Zi = -Zq - distance;
(R.z = Zp - Zi;  -> R.z - Zp = -Zi;)
.sp .5
H1=2*t(1)
H2=2*t(2)
.sp .5
image1: Si = -K12^(u+1)*K23^(m); Zi =  Zq + H1*(n+1) + H2*(m);
image2: Si = -K12^(u+1)*K23^(m); Zi =  Zq - H1*(n+1) - H2*(m);
image3: Si =  K12^(u+1)*K23^(m); Zi = -Zq + H1*(n+1) + H2*(m);
image4: Si =  K12^(u+1)*K23^(m); Zi = -Zq - H1*(n+1) - H2*(m);
image5: Si = -K12^(u)*K23^(m+1); Zi =  Zq + H1*(n)   + H2*(m+1);
image6: Si = -K12^(u)*K23^(m+1); Zi =  Zq - H1*(n)   - H2*(m+1);
image7: Si =  K12^(u)*K23^(m+1); Zi = -Zq + H1*(n)   + H2*(m+1);
image8: Si =  K12^(u)*K23^(m+1); Zi = -Zq - H1*(n)   - H2*(m+1);
.L}
.P
I must conclude that the formules must be:
.P
.L{
           Si                  Zi
-----------------------------------------------------------
 -K12^(1-k+l+n)*K23^k       Zq - 2*(-1+k+l-n)*h1 + 2*k*h2
 -K12^(1-k+l+n)*K23^k       Zq + 2*(-1+k+l-n)*h1 - 2*k*h2
  K12^(1-k+l+n)*K23^k      -Zq - 2*(-1+k+l-n)*h1 + 2*k*h2
  K12^(1-k+l+n)*K23^k      -Zq + 2*(-1+k+l-n)*h1 - 2*k*h2
 -K12^(-k+l+n)*K23^(1+k)    Zq - 2*(k+l-n)*h1 + 2*(1+k)*h2
 -K12^(-k+l+n)*K23^(1+k)    Zq + 2*(k+l-n)*h1 - 2*(1+k)*h2
  K12^(-k+l+n)*K23^(1+k)   -Zq - 2*(k+l-n)*h1 + 2*(1+k)*h2
  K12^(-k+l+n)*K23^(1+k)   -Zq + 2*(k+l-n)*h1 - 2*(1+k)*h2
-----------------------------------------------------------
.L}
