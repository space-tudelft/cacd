network nor3 (terminal A, B, C, F, vss, vdd)
{
    penh w=29.6u l=1.6u (A, v1, vdd);
    penh w=29.6u l=1.6u (B, v1, v2);
    penh w=29.6u l=1.6u (C, v2, F);
    nenh w=23.2u l=1.6u (A, F, vss);
    nenh w=23.2u l=1.6u (B, F, vss);
    nenh w=23.2u l=1.6u (C, F, vss);
}
