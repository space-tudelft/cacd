/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __ROUTING_H
#define __ROUTING_H

/*
 * structures containing global routing information for the partitioned network
 */

typedef struct _ROUTING_CHANNEL
{
   int length;		/* approx. channel length in gridpoints */
   int ncells;		/* width of this channel [# basic cells] */
   int nwires;		/* number of wires in the channel */
}
ROUTING_CHANNEL, *ROUTING_CHANNELPTR;

typedef struct _ROUTING_INFO
{
   ROUTING_CHANNEL channel[2]; /* channel[HOR] and channel[VER] are legal */
}
ROUTING_INFO, *ROUTING_INFOPTR;

typedef struct _GLOBAL_ROUTING
{
   /* each element in the small_channels matrix contains info about the
    * vertical channel to the right of partition [x][y] and the horizontal
    * channel to the top of partition [x][y]:
    */
   ROUTING_INFO **small_channels; /* range is [0..nx-2][0..ny-2] */
   /* following two arrays contain info about channels that partition the
    * entire placement in two pieces. The vertical channel that sits between
    * the partitions [i,i+1][y] is contained in vertical_channel[i] and the
    * info about the horizontal channel that sits between [x][j,j+1] is
    * contained in horizontal_channel[j]. Counting starts at 0:
    */
   ROUTING_CHANNEL *horizontal_channels; /* range is [0..ny-2] */
   ROUTING_CHANNEL *vertical_channels;   /* range is [0..nx-2] */
}
GLOBAL_ROUTING, *GLOBAL_ROUTINGPTR;

#endif
