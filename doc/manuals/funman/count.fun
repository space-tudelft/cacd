#if (CURS == 1)
#include <curses.h>
#endif

#define    CNTNAME    count1
#define    NMBEL    4
#define    CURS     1
#define    CRSLI    4
#define    CRSCO    8
function CNTNAME ( input   phi1, phi2;
                   inread  in[NMBEL], up, load;
                   output  out[NMBEL], tc )
state {
        int      termcount;
        int      count;
}
load {
    cap_add (100e-15, in[0]);
    cap_add (150e-15, in[1]);
    cap_add (200e-15, in[2]);
    cap_add (250e-15, in[3]);
}
initial {
    int i;
    termcount=0;
    count=0;
#if (CURS == 1)
    initscr();
    clear();
    refresh();
#endif
}
behavior {
    delay ('b', 1000 * cap_val(VICIN,MAX,out[0]), out[0]);
    delay ('b', 1000 * cap_val(VICIN,MIN,out[1]), out[1]);
    delay ('b', 1000 * cap_val(VICIN,MAX,out[2]), out[2]);
    delay ('b', 1000 * cap_val(NODE, MAX,out[3]), out[3]);

    single_curs_step();

    if (phi1 == BTTRUE) {
        if (load == BTTRUE) {
            count = BSTOI (in);
        }
        else if (up == BTTRUE) {
            count++;
            if (count == (1<<NMBEL)-1) {
                termcount = 1;
            }
            else
                termcount = 0;
            if (count == (1<<NMBEL)) {
                count = 0;
            }
        }
        else if (up == BTFALSE) {
            count--;
            if (count == -1) {
                count = (1<<NMBEL)-1;
                termcount = 1;
            }
            else
                termcount = 0;
        }
    }
    if (phi2 == BTTRUE) {
        strcpy (out, ITOBS (count, NMBEL));
        if (termcount == 1)
            tc = BTTRUE;
        else
            tc = BTFALSE;
    }
#if (CURS == 1)
    move(CRSLI, CRSCO);
    printw("%s", out);
    printw("%6d", count);
    move(23,0);
    refresh();
#endif
}
