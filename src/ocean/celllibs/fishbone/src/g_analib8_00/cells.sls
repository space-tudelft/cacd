/* * * * * * * * * * * * * * * *
 *
 * Fishbone analog cell library. 8-1993
 * by Anton Bakker, Paul Stravers, Patrick Groeneveld
 *
 * Contents: Contains the sls circuit description
 *           of all cells in the 'analib' library.
 *           See the library description in the
 *           Ontwerppracticumhandleiding (TU Delft
 *           internal report)
 * Purpose:  To create the analog cell library.
 * Created:  july 18th, 1993 by Patrick Groeneveld
 * Modified: --
 * Remarks:  Ignore warings for unconnected terminals
 */

/* * * * *
 *
 * Part 1: Original circuit cells.
 *         These cell may contain functional decrsiptions
 *         to speed up simulation.
 */

network ln3x3 (terminal g, d, s, vss, vdd)
{
    nenh l=3.5u w=69.6u (g, d, s);
}

network lp3x3 (terminal g, d, s, vss, vdd)
{
    penh l=3.6u w=88.8u (g, d, s);
}

network mir_nin (terminal i, g, vss, vdd)
{
    nenh l=3.5u w=69.6u (i, g, i);
    nenh l=3.5u w=69.6u (g, vss, g);
}

network mir_nout (terminal i, g, o, vss, vdd)
{
    nenh l=3.5u w=69.6u (i, o, a);
    nenh l=3.5u w=69.6u (g, a, vss);
}

network mir_pin (terminal i, g, vss, vdd)
{
    penh l=3.5u w=88.8u (i, g, i);
    penh l=3.5u w=88.8u (g, vdd, g);
}

network mir_pout (terminal i, g, o, vss, vdd)
{
    penh l=3.5u w=88.8u (i, o, a);
    penh l=3.5u w=88.8u (g, a, vdd);
}

network osc10 (terminal E, F, XI, XO, vss, vdd)
{
    @ nand tr=100p tf=100p (E, XI, XO);
    @ nand tr=300p tf=400p (E, XO, F);
}


/* * * * * * *
 *
 * Part 2: Extracted cells
 *         These cells start with capitals
 *         They contain only transistors,
 *         resistors and capacitors
 */

network Ln3x3 (terminal g, d, s, vss, vdd)
{
    nenh w=23.2u l=1.6u (60, 7, 41);
    cap 31.59254f (7, vss);
    nenh w=23.2u l=1.6u (61, 7, 14);
    cap 31.59254f (14, vss);
    nenh w=23.2u l=1.6u (59, 14, d);
    nenh w=23.2u l=1.6u (62, 27, d);
    cap 33.03638f (27, vss);
    nenh w=23.2u l=1.6u (58, 27, 34);
    cap 33.03638f (34, vss);
    nenh w=23.2u l=1.6u (63, 34, s);
    res 183.0893 (41, s);
    cap 50.78461f (41, vss);
    cap 41.23057f (s, vss);
    nenh w=23.2u l=1.6u (57, s, 48);
    cap 31.59254f (48, vss);
    nenh w=23.2u l=1.6u (64, 48, 55);
    cap 31.59254f (55, vss);
    nenh w=23.2u l=1.6u (56, 55, d);
    res 140.6821 (g, 56);
    res 143.2366 (g, 57);
    res 149.927 (g, 58);
    res 149.9249 (g, 59);
    res 150.6297 (g, 60);
    res 149.9249 (g, 61);
    res 149.927 (g, 62);
    res 149.927 (g, 63);
    res 143.2366 (g, 64);
    cap 34.26446f (g, vss);
    cap 4.539861f (56, vss);
    cap 4.544917f (57, vss);
    cap 4.848077f (58, vss);
    cap 4.35723f (59, vss);
    cap 4.352508f (60, vss);
    cap 4.35723f (61, vss);
    cap 4.848077f (62, vss);
    cap 4.848077f (63, vss);
    cap 4.544917f (64, vss);
    cap 81.16528f (d, vss);
    res 497.0514 (vss, 77);
    cap 41.12208f (vss, vss);
    cap 2.950429f (77, vss);
    cap 36.83792f (vdd, vss);
}

network Lp3x3 (terminal g, d, s, vss, vdd)
{
    penh w=29.6u l=1.6u (62, 9, 43);
    cap 90.25434f (9, vss);
    penh w=29.6u l=1.6u (63, 9, 16);
    cap 90.25434f (16, vss);
    penh w=29.6u l=1.6u (61, 16, s);
    penh w=29.6u l=1.6u (64, 29, s);
    cap 91.69818f (29, vss);
    penh w=29.6u l=1.6u (60, 29, 36);
    cap 91.69818f (36, vss);
    penh w=29.6u l=1.6u (65, 36, d);
    res 192.1254 (d, 43);
    cap 101.0897f (d, vss);
    cap 127.5971f (43, vss);
    penh w=29.6u l=1.6u (59, d, 50);
    cap 90.25434f (50, vss);
    penh w=29.6u l=1.6u (66, 50, 57);
    cap 90.25434f (57, vss);
    penh w=29.6u l=1.6u (58, 57, s);
    res 597.7727 (g, 58);
    res 598.3152 (g, 59);
    res 598.3883 (g, 60);
    res 598.1335 (g, 61);
    res 598.6038 (g, 62);
    res 598.1335 (g, 63);
    res 598.3883 (g, 64);
    res 598.3883 (g, 65);
    res 598.3152 (g, 66);
    cap 47.41822f (g, vss);
    cap 3.111008f (58, vss);
    cap 3.114514f (59, vss);
    cap 3.259562f (60, vss);
    cap 3.028629f (61, vss);
    cap 3.028149f (62, vss);
    cap 3.028629f (63, vss);
    cap 3.259562f (64, vss);
    cap 3.259562f (65, vss);
    cap 3.114514f (66, vss);
    cap 201.0074f (s, vss);
    cap 36.66864f (vss, vss);
    res 137.084 (vdd, 81);
    cap 40.13621f (vdd, vss);
    cap 4.092584f (81, vss);
}

network Mir_nin (terminal i, g, vss, vdd)
{
    nenh w=23.2u l=1.6u (61, 7, i);
    cap 30.1487f (7, vss);
    nenh w=23.2u l=1.6u (63, 7, 14);
    cap 30.1487f (14, vss);
    nenh w=23.2u l=1.6u (60, 14, g);
    nenh w=23.2u l=1.6u (58, 27, g);
    cap 31.59254f (27, vss);
    nenh w=23.2u l=1.6u (59, 27, 34);
    cap 31.59254f (34, vss);
    nenh w=23.2u l=1.6u (62, 34, i);
    nenh w=23.2u l=1.6u (56, 47, i);
    cap 31.59254f (47, vss);
    nenh w=23.2u l=1.6u (57, 47, 54);
    cap 31.59254f (54, vss);
    nenh w=23.2u l=1.6u (55, 54, g);
    res 140.6821 (i, 55);
    res 140.4388 (i, 56);
    res 143.2366 (i, 59);
    res 141.0979 (i, 60);
    res 139.2399 (i, 61);
    res 141.0979 (i, 63);
    res 143.2366 (i, 58);
    res 143.2366 (i, 62);
    res 143.2366 (i, 57);
    cap 115.0666f (i, vss);
    cap 4.539861f (55, vss);
    cap 4.578032f (56, vss);
    cap 4.544917f (57, vss);
    cap 4.544917f (58, vss);
    cap 4.544917f (59, vss);
    cap 4.089673f (60, vss);
    cap 4.120188f (61, vss);
    cap 4.544917f (62, vss);
    cap 4.089673f (63, vss);
    nenh w=23.2u l=1.6u (131, 76, g);
    cap 31.59254f (76, vss);
    nenh w=23.2u l=1.6u (130, 76, 83);
    cap 33.76926f (83, vss);
    nenh w=23.2u l=1.6u (129, 83, vss);
    nenh w=23.2u l=1.6u (127, 96, vss);
    cap 31.59254f (96, vss);
    nenh w=23.2u l=1.6u (128, 96, 103);
    cap 31.59254f (103, vss);
    nenh w=23.2u l=1.6u (126, 103, g);
    nenh w=23.2u l=1.6u (125, 116, g);
    cap 30.1487f (116, vss);
    nenh w=23.2u l=1.6u (132, 116, 123);
    cap 30.1487f (123, vss);
    nenh w=23.2u l=1.6u (124, 123, vss);
    res 138.5422 (g, 124);
    res 141.0979 (g, 125);
    res 143.2366 (g, 128);
    res 140.4388 (g, 129);
    res 143.2366 (g, 130);
    res 143.1658 (g, 131);
    res 143.2366 (g, 127);
    res 143.2366 (g, 126);
    res 141.0979 (g, 132);
    cap 161.5651f (g, vss);
    cap 4.0858f (124, vss);
    cap 4.089673f (125, vss);
    cap 4.544917f (126, vss);
    cap 5.592422f (127, vss);
    cap 4.544917f (128, vss);
    cap 4.578032f (129, vss);
    cap 4.544917f (130, vss);
    cap 4.535727f (131, vss);
    cap 4.089673f (132, vss);
    res 495.7327 (vss, 145);
    cap 143.8494f (vss, vss);
    cap 2.956572f (145, vss);
    cap 67.30832f (vdd, vss);
}

network Mir_nout (terminal i, g, o, vss, vdd)
{
    nenh w=23.2u l=1.6u (60, 7, 41);
    cap 31.59254f (7, vss);
    nenh w=23.2u l=1.6u (61, 7, 14);
    cap 31.59254f (14, vss);
    nenh w=23.2u l=1.6u (59, 14, 111);
    nenh w=23.2u l=1.6u (62, 27, 111);
    cap 33.03638f (27, vss);
    nenh w=23.2u l=1.6u (58, 27, 34);
    cap 33.03638f (34, vss);
    nenh w=23.2u l=1.6u (63, 34, o);
    res 181.3259 (o, 41);
    cap 41.14055f (o, vss);
    cap 50.73513f (41, vss);
    nenh w=23.2u l=1.6u (57, o, 48);
    cap 31.59254f (48, vss);
    nenh w=23.2u l=1.6u (64, 48, 55);
    cap 31.59254f (55, vss);
    nenh w=23.2u l=1.6u (56, 55, 111);
    res 147.3737 (i, 56);
    res 149.9249 (i, 57);
    res 149.9251 (i, 58);
    res 141.2747 (i, 59);
    res 141.9792 (i, 60);
    res 141.2747 (i, 61);
    res 149.9251 (i, 62);
    res 149.9251 (i, 63);
    res 149.9249 (i, 64);
    cap 34.09483f (i, vss);
    cap 4.349105f (56, vss);
    cap 4.35723f (57, vss);
    cap 4.84813f (58, vss);
    cap 4.60293f (59, vss);
    cap 4.596703f (60, vss);
    cap 4.60293f (61, vss);
    cap 4.84813f (62, vss);
    cap 4.84813f (63, vss);
    cap 4.35723f (64, vss);
    nenh w=23.2u l=1.6u (130, 77, 111);
    cap 31.59254f (77, vss);
    nenh w=23.2u l=1.6u (131, 77, 84);
    cap 31.59254f (84, vss);
    nenh w=23.2u l=1.6u (129, 84, vss);
    nenh w=23.2u l=1.6u (132, 97, vss);
    cap 31.59254f (97, vss);
    nenh w=23.2u l=1.6u (128, 97, 104);
    cap 31.59254f (104, vss);
    nenh w=23.2u l=1.6u (133, 104, 111);
    cap 126.365f (111, vss);
    nenh w=23.2u l=1.6u (127, 111, 118);
    cap 30.1487f (118, vss);
    nenh w=23.2u l=1.6u (134, 118, 125);
    cap 30.1487f (125, vss);
    nenh w=23.2u l=1.6u (126, 125, vss);
    res 138.5422 (g, 126);
    res 141.0979 (g, 127);
    res 149.9249 (g, 128);
    res 149.9249 (g, 129);
    res 150.6297 (g, 130);
    res 149.9249 (g, 131);
    res 149.9249 (g, 132);
    res 149.9249 (g, 133);
    res 141.0979 (g, 134);
    cap 34.031f (g, vss);
    cap 4.0858f (126, vss);
    cap 4.089673f (127, vss);
    cap 4.35723f (128, vss);
    cap 4.35723f (129, vss);
    cap 4.352508f (130, vss);
    cap 4.35723f (131, vss);
    cap 5.362361f (132, vss);
    cap 4.35723f (133, vss);
    cap 4.089673f (134, vss);
    res 495.7327 (vss, 147);
    cap 143.8494f (vss, vss);
    cap 2.956572f (147, vss);
    cap 67.30832f (vdd, vss);
}

network Mir_pin (terminal i, g, vss, vdd)
{
    penh w=29.6u l=1.6u (65, 9, i);
    cap 88.8105f (9, vss);
    penh w=29.6u l=1.6u (63, 9, 16);
    cap 88.8105f (16, vss);
    penh w=29.6u l=1.6u (60, 16, g);
    penh w=29.6u l=1.6u (62, 29, g);
    cap 90.25434f (29, vss);
    penh w=29.6u l=1.6u (64, 29, 36);
    cap 90.25434f (36, vss);
    penh w=29.6u l=1.6u (61, 36, i);
    penh w=29.6u l=1.6u (58, 49, i);
    cap 90.25434f (49, vss);
    penh w=29.6u l=1.6u (59, 49, 56);
    cap 90.25434f (56, vss);
    penh w=29.6u l=1.6u (57, 56, g);
    res 597.7727 (i, 57);
    res 599.2217 (i, 58);
    res 598.3152 (i, 61);
    res 598.3152 (i, 62);
    res 597.9395 (i, 63);
    res 599.5777 (i, 65);
    res 597.9395 (i, 60);
    res 598.3152 (i, 64);
    res 598.3152 (i, 59);
    cap 264.099f (i, vss);
    cap 3.111008f (57, vss);
    cap 3.123546f (58, vss);
    cap 3.114514f (59, vss);
    cap 2.883997f (60, vss);
    cap 3.114514f (61, vss);
    cap 3.114514f (62, vss);
    cap 2.883997f (63, vss);
    cap 3.114514f (64, vss);
    cap 2.892854f (65, vss);
    penh w=29.6u l=1.6u (129, 78, g);
    cap 90.25434f (78, vss);
    penh w=29.6u l=1.6u (133, 78, 85);
    cap 90.25434f (85, vss);
    penh w=29.6u l=1.6u (132, 85, vdd);
    penh w=29.6u l=1.6u (128, 98, vdd);
    cap 90.25434f (98, vss);
    penh w=29.6u l=1.6u (130, 98, 105);
    cap 90.25434f (105, vss);
    penh w=29.6u l=1.6u (131, 105, g);
    penh w=29.6u l=1.6u (127, 118, g);
    cap 88.8105f (118, vss);
    penh w=29.6u l=1.6u (134, 118, 125);
    cap 88.8105f (125, vss);
    penh w=29.6u l=1.6u (126, 125, vdd);
    res 597.3969 (g, 126);
    res 597.9395 (g, 127);
    res 598.3152 (g, 130);
    res 598.3152 (g, 133);
    res 598.7855 (g, 129);
    res 598.3152 (g, 132);
    res 599.2217 (g, 128);
    res 598.3152 (g, 131);
    res 597.9395 (g, 134);
    cap 354.5025f (g, vss);
    cap 2.880755f (126, vss);
    cap 2.883997f (127, vss);
    cap 4.191287f (128, vss);
    cap 3.113967f (129, vss);
    cap 3.114514f (130, vss);
    cap 3.114514f (131, vss);
    cap 3.114514f (132, vss);
    cap 3.114514f (133, vss);
    cap 2.883997f (134, vss);
    cap 67.13904f (vss, vss);
    res 137.0994 (vdd, 149);
    cap 265.4289f (vdd, vss);
    cap 4.124227f (149, vss);
}

network Mir_pout (terminal i, g, o, vss, vdd)
{
    penh w=29.6u l=1.6u (62, 9, 43);
    cap 90.25434f (9, vss);
    penh w=29.6u l=1.6u (63, 9, 16);
    cap 90.25434f (16, vss);
    penh w=29.6u l=1.6u (61, 16, 113);
    penh w=29.6u l=1.6u (64, 29, 113);
    cap 91.69818f (29, vss);
    penh w=29.6u l=1.6u (60, 29, 36);
    cap 91.69818f (36, vss);
    penh w=29.6u l=1.6u (65, 36, o);
    res 189.3639 (43, o);
    cap 127.3772f (43, vss);
    cap 101.5662f (o, vss);
    penh w=29.6u l=1.6u (59, o, 50);
    cap 90.25434f (50, vss);
    penh w=29.6u l=1.6u (66, 50, 57);
    cap 90.25434f (57, vss);
    penh w=29.6u l=1.6u (58, 57, 113);
    res 597.591 (i, 58);
    res 598.1335 (i, 59);
    res 605.5328 (i, 60);
    res 605.4285 (i, 61);
    res 605.8988 (i, 62);
    res 605.4285 (i, 63);
    res 605.5328 (i, 64);
    res 605.5328 (i, 65);
    res 598.1335 (i, 66);
    cap 46.10376f (i, vss);
    cap 3.025044f (58, vss);
    cap 3.028629f (59, vss);
    cap 3.47783f (60, vss);
    cap 3.334504f (61, vss);
    cap 3.333793f (62, vss);
    cap 3.334504f (63, vss);
    cap 3.47783f (64, vss);
    cap 3.47783f (65, vss);
    cap 3.028629f (66, vss);
    penh w=29.6u l=1.6u (132, 79, 113);
    cap 90.25434f (79, vss);
    penh w=29.6u l=1.6u (133, 79, 86);
    cap 90.25434f (86, vss);
    penh w=29.6u l=1.6u (131, 86, vdd);
    penh w=29.6u l=1.6u (134, 99, vdd);
    cap 90.25434f (99, vss);
    penh w=29.6u l=1.6u (130, 99, 106);
    cap 90.25434f (106, vss);
    penh w=29.6u l=1.6u (135, 106, 113);
    cap 306.0199f (113, vss);
    penh w=29.6u l=1.6u (129, 113, 120);
    cap 88.8105f (120, vss);
    penh w=29.6u l=1.6u (136, 120, 127);
    cap 88.8105f (127, vss);
    penh w=29.6u l=1.6u (128, 127, vdd);
    res 597.3969 (g, 128);
    res 597.9395 (g, 129);
    res 598.1335 (g, 130);
    res 598.1335 (g, 131);
    res 598.6038 (g, 132);
    res 598.1335 (g, 133);
    res 598.1335 (g, 134);
    res 598.1335 (g, 135);
    res 597.9395 (g, 136);
    cap 45.66866f (g, vss);
    cap 2.880755f (128, vss);
    cap 2.883997f (129, vss);
    cap 3.028629f (130, vss);
    cap 3.028629f (131, vss);
    cap 3.028149f (132, vss);
    cap 3.028629f (133, vss);
    cap 4.096869f (134, vss);
    cap 3.028629f (135, vss);
    cap 2.883997f (136, vss);
    cap 67.13904f (vss, vss);
    res 137.0994 (vdd, 151);
    cap 262.6801f (vdd, vss);
    cap 4.124227f (151, vss);
}

network Osc10 (terminal E, F, XI, XO, vss, vdd)
{
    nenh w=0.00017 l=1.6u (E, 6, XO);
    penh w=0.0002 l=1.6u (XI, XO, vdd);
    nenh w=0.00017 l=1.6u (XI, 6, vss);
    penh w=0.0002 l=1.6u (E, XO, vdd);
    nenh w=23.2u l=1.6u (E, 7, vss);
    penh w=29.6u l=1.6u (E, F, vdd);
    nenh w=23.2u l=1.6u (XO, 7, F);
    penh w=29.6u l=1.6u (XO, F, vdd);
    cap 40f (7, vss);
    cap 150f (F, vss);
}
