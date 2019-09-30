#define NMBIN    2     /* number of 'numbers' that are multiplied */
#define NMBBTS   8     /* number of bits for an input number */
#define NMBOUT   16    /* number of bits for an output number */

function mul8x8 (input 	in[NMBIN][NMBBTS];
		 output out[NMBOUT])

load {
	int i, j;
	for ( i=0 ; i<NMBIN ; i++ ) {
		for ( j=0 ; j<NMBBTS ; j++ ) {
			cap_add (100e-15, in[i][j]);
		        /* assign 100fF to each input node */
		}
	}
}


initial {
	int i;
	for ( i=0 ; i<NMBOUT ; i++ ) {
		delay ('r', 2e-9, out[i]);
		delay ('f', 2e-9, out[i]);
	}
}

behavior {
	int i;
	int outint=1;

	for ( i=0 ; i<NMBIN ; i++ ) {
		if (strchr (in[i], 'X'))
			break;
	}

	if ( i<NMBIN ) {
		BSUNDEF (out);       /* There was an X in the input */
	}
	else {
		for ( i=0 ; i<NMBIN ; i++ ) {
			outint = BSTOI (in[i]) * outint;
		}

		strcpy (out, ITOBS (outint,NMBOUT ) );
	}
}
