network total (terminal phi1, phi2, sel[0..3], sx, sy, readwrite,
	       in[0..7], out[0..15])
{
    net {(x[0..7])};
    net {(y[0..7])};
    net {(k[0..7])};

    @ ram (sel[0..3], in[0..7], k[0..7], readwrite, phi1, phi2);

    nenh w=3u l=1u (sx, k[0], x[0]);
    nenh w=3u l=1u (sx, k[1], x[1]);
    nenh w=3u l=1u (sx, k[2], x[2]);
    nenh w=3u l=1u (sx, k[3], x[3]);
    nenh w=3u l=1u (sx, k[4], x[4]);
    nenh w=3u l=1u (sx, k[5], x[5]);
    nenh w=3u l=1u (sx, k[6], x[6]);
    nenh w=3u l=1u (sx, k[7], x[7]);

    nenh w=3u l=1u (sy, k[0], y[0]);
    nenh w=3u l=1u (sy, k[1], y[1]);
    nenh w=3u l=1u (sy, k[2], y[2]);
    nenh w=3u l=1u (sy, k[3], y[3]);
    nenh w=3u l=1u (sy, k[4], y[4]);
    nenh w=3u l=1u (sy, k[5], y[5]);
    nenh w=3u l=1u (sy, k[6], y[6]);
    nenh w=3u l=1u (sy, k[7], y[7]);

    @ mul8x8 (x[0..7], y[0..7], out[0..15]);
}
