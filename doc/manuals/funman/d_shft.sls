network inverter (terminal vdd, vss, i, o)
{
    nenh w=8u l=4u (i, o, vss);
    ndep w=4u l=8u (o, vdd, o);
}

network doub_shft (terminal vdd, vss, in[1..4], irA, ilA, cA0, cA1, irB, ilB,
                            cB0, cB1, enableA, enableB, bus[1..4], phi1, phi2)
{
    {instA} @ shiftregister (irA, ilA, cA0, cA1, enableA,
                             bus[1..4], phi1, phi2);
    {instB} @ shiftregister (irB, ilB, cB0, cB1, enableB,
                             bus[1..4], phi1, phi2);

    {inst_inv[1..4]} inverter (vdd, vss, in[1], bus[1],
                               vdd, vss, in[2], bus[2],
                               vdd, vss, in[3], bus[3],
                               vdd, vss, in[4], bus[4]);
}
