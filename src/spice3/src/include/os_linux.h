/*
 *	Linux/GNU
 */

#include "os_unix.h"

#define HAS_POSIXTTY		/* <termios.h> */
#define HAS_FLOAT_H
#define HAS_FTIME		/* ftime(), <times.h> */
#define HAS_BSDRLIMIT
#define HAS_SYSVRUSAGE
#define HAS_BCOPY		/* bcopy(), bzero() */
#define HAS_BSDRANDOM		/* srandom() and random() */
#define HAS_BSDSOCKETS		/* <net/inet.h>, socket(), etc. */
#define HAS_INTWAITSTATUS	/* wait(3) takes an int *, not a union */

