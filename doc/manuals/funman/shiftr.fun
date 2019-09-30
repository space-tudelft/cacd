#define  NAME   shiftregister
#define  NMBEL  4

function NAME ( inread  ir, il, c0, c1, o_enable;
                inout   in_out[NMBEL];
                input   phi1, phi2 )

state {
    char  mem[NMBEL];
}

behavior {
    int i;
    if (phi1 == BTTRUE) {
        if (c0 == BTTRUE && c1 == BTTRUE)         /* load */
            BSCOPY (mem, in_out);
        else if (c0 == BTTRUE && c1 == BTFALSE) { /* shift left */
            for (i = NMBEL-1; i > 0; i--)
                mem[i] = mem[i-1];
            mem[0] = ir;
        }
        else if (c0 == BTFALSE && c1 == BTTRUE) { /* shift right */
            for (i = 0; i < NMBEL-1; i++)
                mem[i] = mem[i+1];
            mem[NMBEL-1] = il;
        }
    }
    BSFREE (in_out);
    if (phi2 == BTTRUE && o_enable == BTTRUE) /* write */
        BSCOPY (in_out, mem);
}
