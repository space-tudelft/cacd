# directory: demo/FreePDK45/ringosc

model NMOS_VTL nmos ( level=5

# parameters related to the technology node
eta0 = 0.0049
cgso = 1.1e-10
cgdo = 1.1e-10

# parameters customized by the user
k1 = 0.74

# secondary parameters
xpart= 0
k2   = 0.01
etab = 0
vfb  = -0.55
rsh  = 5
cgbo = 2.56e-011
)

model PMOS_VTL pmos ( level=5

# parameters related to the technology node
eta0 = 0.0049
cgso = 1.1e-10
cgdo = 1.1e-10

# parameters customized by the user
k1 = 0.694

# secondary parameters
xpart= 0
k2   = -0.01
etab = 0
vfb  = 0.55
rsh  = 5
cgbo = 2.56e-011
)

