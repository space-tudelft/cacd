function multiplexer ( input   in[4];
                       output  out;
                       input   select[2] )

behavior {
    if ( BSTOI(select) < 4 )
        out = in[ BSTOI(select) ];
}
