#define NMBADRB   4       /* number of address bits */
#define NMBADRSS  16      /* number of words */
#define NMBDTA    8       /* word length */

function ram (	inread 	addr[NMBADRB], in[NMBDTA];
      		output 	out[NMBDTA];
      		inread 	rw;
      		input  	phi1, phi2)

state {
    char mem[NMBADRSS][NMBDTA];
}

initial {
    int i;
    for (i=0 ; i<NMBDTA ; i++ ) {
    	delay ('r', 1.5e-9, out[i]);
    	delay ('f', 1.5e-9, out[i]);
    }
}

behavior {
    int i;

    if (phi1 == 'I' && rw == 'O') {
        strcpy (mem[ BSTOI (addr) ], in);     /* read */
    }

    if (phi2 == 'I' && rw == 'I') {
        strcpy (out, mem[ BSTOI (addr) ]);    /* write */
    }
}
