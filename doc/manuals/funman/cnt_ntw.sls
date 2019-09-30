network invertor (terminal vdd, vss, i, o)
{
    nenh  w=16u l=4u  (i, o, vss);
    ndep  w=8u  l=8u  (o, vdd, o);
}

network count_netw (terminal vdd, vss, phi1, phi2, in[1..4], up, load, p[1..5],
                                out[1..4], tc, in_in[1..4])
{
    {inst1} @ count1 (phi1, phi2, in_in[1..4], up, load, out[1..4], tc);

    {inst_inv[1..4]} invertor (vdd, vss, in[1], in_in[1],
                               vdd, vss, in[2], in_in[2],
                               vdd, vss, in[3], in_in[3],
                               vdd, vss, in[4], in_in[4]);

    cap 150f (out[1], gnd);
    cap 200f (out[2], gnd);
    cap 250f (out[3], gnd);
    cap 100f (out[4], gnd);

    cap 300f (o1, gnd);
    cap 150f (o2, gnd);
    cap 250f (o3, gnd);
    cap 200f (o4, gnd);

    cap 200f (1, gnd);
    cap 250f (2, gnd);
    cap 100f (3, gnd);
    cap 150f (4, gnd);
    cap 150f (5, gnd);

    nenh w=8u l=4u (p[1], out[1], 1);
    nenh w=8u l=4u (p[2], out[2], 3);
    nenh w=8u l=4u (p[3], 3, o2);
    nenh w=8u l=4u (p[4], 4, o3);
    nenh w=8u l=4u (p[5], out[3], 5);

    @ nand (3, 2, o1);

    res 20k (1, 2);
    res 10k (out[3], 4);
    res 10k (5, o4);
}
