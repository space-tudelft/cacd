RC 1 2 3 4 5

*
* circuit RC 1 2 3 4 5
*

.subckt RCD a b
.include RC_100G.spc
.ends RCD

.subckt RCC a b
.include RC_1G.spc
.ends RCC

.subckt RCB a b
.include RC_100M.spc
.ends RCB

.subckt RCA a b
.include RC_0.spc
.ends RCA


x1 1 f0 RCA
x2 1 f100M RCB
x3 1 f1G RCC
x4 1 f100G RCD

v1 1 0 dc 0 ac 1

r1 f0 0 100meg
r2 f100M 0 100meg
r3 f1G 0 100meg
r4 f100G 0 100meg

.control
ac dec 10 1k 999G
plot vdb(f0),vdb(f100M),vdb(f1G),vdb(f100G) ylimit -60 0
plot vp(f0),vp(f100M),vp(f1G),vp(f100G)
.endc

.end

