letsee (s1, s2, s3, s4, fun)
char s1, s2, s3, s4;
char fun[];
{
    char dum;
    dum = BTEXOR (s1, s2, s3, BTTRUE);
    fprintf (stderr,
            "e_i[0]=%c, e_i[1]=%c, e_i[2]=%c, e_i[3]=I, exorresult=%c\en",
            s1, s2, s3, dum);
    fprintf (stderr, "  i[0]=%c,   i[1]=%c,   i[2]=%c, %sresult=%c\en",
            s1, s2, s3, fun, s4);
}

char four_of_four (s1, s2, s3, s4)
char s1, s2, s3, s4;
{
    if (s1 == BTUNDEF || s2 == BTUNDEF || s3 == BTUNDEF || s4 == BTUNDEF)
        return (BTUNDEF);
    else if (s1 == BTFALSE && s2 == BTTRUE && s3 == BTFALSE && s4 == BTFALSE)
        return (BTTRUE);
    else
        return (BTFALSE);
}


function combinator ( input  phi;
                      output oinv, oand, onand, oor,
                             onor, oexor, oexnor, osp[2], us_in[4] )

behavior {
    char in[5];

    if (phi == BTTRUE) {
        printf ("typ input (4 bits): ");
        scanf ("%4s", in);
        oinv   = BTINVERT (in[0]);
        oand   = BTAND (in[0], in[1], in[2]);
        letsee (in[0], in[1], in[2], oand, "AND");
        onand  = BTNAND (in[0], in[1], in[2], in[3]);
        oor    = BTOR (in[0], in[1], in[2]);
        letsee (in[0], in[1], in[2], oor, "OR");
        onor   = BTNOR (in[0], in[1], in[2], in[3]);
        oexor  = BTEXOR (in[0], in[1], in[2]);
        oexnor = BTEXNOR (in[0], in[1], in[2], in[3]);
        osp[0] = BTEXOR (BTNOR (BTAND (BTEXNOR (in[2], in[3]), in[1]), 
                                  in[0]), in[1], in[2]);
        osp[1] = BTEXNOR (four_of_four (in[0], BTINVERT (in[1]), in[2], in[3]),  
                          BTAND (BTEXNOR (in[2], in[3]), in[1]), in[0],
                                   in[1], in[2]);
        BSCOPY (us_in, in);
    }
}
