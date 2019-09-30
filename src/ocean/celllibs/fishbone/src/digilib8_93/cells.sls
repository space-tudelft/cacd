/* * * * * * * * * * * * * * * *
 *
 * Fishbone digital basic cell library. 8-1993
 * by Anton Bakker,  Patrick Groeneveld, Paul Stravers
 *
 * Contents: Contains the sls circuit description
 *           of all cells in the 'digilib8_93' library.
 *           See the library description in the
 *           Ontwerppracticumhandleiding (TU Delft
 *           internal report)
 * Purpose:  To create the oplib library.
 * Created:  july 18, 1993 by Patrick Groeneveld
 * Modified:
 * Remark:   ingore the warnings of inconnected vss and
 *           vdd terminals
 */

/* * * * *
 *
 * Part 1: Original circuit cells.
 *         These cells ar built with functional descriptions,
 *         to speed up simulation.
 */
network buf40 (terminal A, Y, vss, vdd)
{
    @ and tr=700p tf=850p (A, Y);
}

network de211 (terminal A, B, Y0, Y1, Y2, Y3, vss, vdd)
{
    @ invert tr=300p tf=300p (A, an);
    @ invert tr=300p tf=300p (B, bn);
    @ nor tr=300p tf=200p (A, B, Y0);
    @ nor tr=300p tf=200p (an, B, Y1);
    @ nor tr=300p tf=200p (A, bn, Y2);
    @ nor tr=300p tf=200p (an, bn, Y3);
}

network de310 (terminal A, B, C, Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7, vss, vdd)
{
    @ invert tr=500p tf=500p (A, an);
    @ invert tr=500p tf=500p (B, bn);
    @ invert tr=500p tf=500p (C, cn);
    @ nor tr=300p tf=200p (A, B, C, Y0);
    @ nor tr=300p tf=200p (an, B, C, Y1);
    @ nor tr=300p tf=200p (A, bn, C, Y2);
    @ nor tr=300p tf=200p (an, bn, C, Y3);
    @ nor tr=300p tf=200p (A, B, cn, Y4);
    @ nor tr=300p tf=200p (an, B, cn, Y5);
    @ nor tr=300p tf=200p (A, bn, cn, Y6);
    @ nor tr=300p tf=200p (an, bn, cn, Y7);
}

network dfn10 (terminal D, CK, Q, vss, vdd)
{
    @ invert (D, dn);
    @ invert (CK, ckn);
    @ nand (D, ckn, 1);
    @ nand (dn, ckn, 2);
    @ nand tr=500p tf=500p (1, 4, 3);
    @ nand (2, 3, 4);
    @ nand (3, CK, 5);
    @ nand (4, CK, 6);
    @ nand tr=2.1n tf=1.9n (5, 7, Q);
    @ nand (6, Q, 7);
}

network dfr11 (terminal D, R, CK, Q, vss, vdd)
{
    @ invert (R, rn);
    @ and (rn, D, d2);
    @ invert (d2, d2n);
    @ invert (CK, ckn);
    @ nand (d2, ckn, 1);
    @ nand (d2n, ckn, 2);
    @ nand tr=500p tf=500p (1, 4, 3);
    @ nand (2, 3, 4);
    @ nand (3, CK, 5);
    @ nand (4, CK, 6);
    @ nand tr=2.1n tf=1.9n (5, 7, Q);
    @ nand (6, Q, 7);
}

network dfa11 (terminal D, R, CK, Q, vss, vdd)
{
    @ invert (D, dn);
    @ nor (CK, R, ckn);
    @ invert (ckn, cknn);
    @ nand (D, ckn, 1);
    @ nand (dn, ckn, 2);
    @ nand tr=500p tf=500p (1, 4, 3);
    @ nand (2, 3, 4);
    @ nand (3, cknn, 5);
    @ nand (4, cknn, 6);
    @ nand tr=2.1n tf=1.9n (5, 7, Q);
    @ nand (6, Q, 7);
}

network ex210 (terminal A, B, Y, vss, vdd)
{
    @ exor tr=700p tf=400p (A, B, Y);
}

network iv110 (terminal A, Y, vss, vdd)
{
    @ invert tr=190p tf=230p (A, Y);
}

network mu111 (terminal A, B, S, Y, vss, vdd)
{
    @ invert (S, sn);
    @ and (A, sn, 1);
    @ and (B, S, 2);
    @ or tr=600p tf=700p (1, 2, Y);
}

network mu210 (terminal S1, S2, A, B, C, D, Y, vss, vdd)
{
    @ invert (S1, s1n);
    @ invert (S2, s2n);
    @ and (s1n, s2n, A, 1);
    @ and (S1, s2n, B, 2);
    @ and (s1n, S2, C, 3);
    @ and (S1, S2, D, 4);
    @ or tr=1.2n tf=1.4n (1, 2, 3, 4, Y);
}

network na210 (terminal A, B, Y, vss, vdd)
{
    @ nand tr=300p tf=300p (A, B, Y);
}

network na310 (terminal A, B, C, Y, vss, vdd)
{
    @ nand tr=300p tf=400p (A, B, C, Y);
}

network no210 (terminal A, B, Y, vss, vdd)
{
    @ nor tr=330p tf=240p (A, B, Y);
}

network no310 (terminal A, B, C, Y, vss, vdd)
{
    @ nor tr=500p tf=260p (A, B, C, Y);
}


/* * * * * * *
 *
 * Part 2: Extracted cells
 *         These cells start with capitals.
 *         They contain only transistors,
 *         resistors and capacitors
 */

network Buf40 (terminal A, Y, vss, vdd)
{
    nenh w=23.2u l=1.6u (A, 17, vss);
    penh w=29.6u l=1.6u (A, 17, vdd);
    nenh l=1.6u w=92.8u (17, Y, vss);
    penh l=1.6u w=118.4u (17, Y, vdd);
    cap 15.69586f (A, vss);
    cap 244.9571f (17, vss);
    cap 274.0332f (Y, vss);
    cap 144.3266f (vss, vss);
    cap 325.5773f (vdd, vss);
}

network De211  (terminal A, B, Y0, Y1, Y2, Y3, vss, vdd)
{
    nenh w=23.2u l=1.6u (B, 39, vss);
    penh w=29.6u l=1.6u (B, 39, vdd);
    nenh w=23.2u l=1.6u (B, Y0, vss);
    penh w=29.6u l=1.6u (B, 17, vdd);
    nenh w=23.2u l=1.6u (A, Y0, vss);
    penh w=29.6u l=1.6u (A, Y0, 17);
    nenh w=23.2u l=1.6u (B, Y1, vss);
    cap 69.78283f (B, vss);
    cap 145.9012f (Y0, vss);
    nenh w=23.2u l=1.6u (34, Y1, vss);
    cap 200.1232f (17, vss);
    penh w=29.6u l=1.6u (34, 17, Y1);
    cap 144.4574f (Y1, vss);
    nenh w=23.2u l=1.6u (A, 34, vss);
    penh w=29.6u l=1.6u (A, 34, vdd);
    nenh w=23.2u l=1.6u (39, Y3, vss);
    penh w=29.6u l=1.6u (39, 44, vdd);
    nenh w=23.2u l=1.6u (34, Y3, vss);
    penh w=29.6u l=1.6u (34, Y3, 44);
    cap 198.1908f (34, vss);
    nenh w=23.2u l=1.6u (39, Y2, vss);
    cap 236.6849f (39, vss);
    cap 140.1447f (Y3, vss);
    nenh w=23.2u l=1.6u (A, Y2, vss);
    cap 198.6794f (44, vss);
    penh w=29.6u l=1.6u (A, 44, Y2);
    cap 100.3143f (A, vss);
    cap 137.2571f (Y2, vss);
    cap 296.7671f (vss, vss);
    cap 279.1554f (vdd, vss);
}

network De310 (terminal A, B, C, Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7, vss, vdd)
{
    nenh w=23.2u l=1.6u (B, 8, vss);
    penh w=29.6u l=1.6u (B, 8, vdd);
    penh w=29.6u l=1.6u (C, 15, vdd);
    nenh w=23.2u l=1.6u (C, 15, vss);
    nenh w=23.2u l=1.6u (15, Y6, vss);
    penh w=29.6u l=1.6u (15, 1, vdd);
    nenh w=23.2u l=1.6u (8, Y6, vss);
    cap 98.75936f (1, vss);
    penh w=29.6u l=1.6u (8, 1, 2);
    penh w=29.6u l=1.6u (C, 3, vdd);
    nenh w=23.2u l=1.6u (C, Y2, vss);
    nenh w=23.2u l=1.6u (A, Y6, vss);
    cap 98.04864f (2, vss);
    penh w=29.6u l=1.6u (A, 2, Y6);
    cap 93.71712f (3, vss);
    penh w=29.6u l=1.6u (8, 3, 4);
    nenh w=23.2u l=1.6u (8, Y2, vss);
    cap 184.5916f (Y6, vss);
    cap 97.5136f (4, vss);
    penh w=29.6u l=1.6u (A, 4, Y2);
    nenh w=23.2u l=1.6u (A, Y2, vss);
    nenh w=23.2u l=1.6u (15, Y7, vss);
    penh w=29.6u l=1.6u (15, 5, vdd);
    cap 181.6687f (Y2, vss);
    nenh w=23.2u l=1.6u (8, Y7, vss);
    cap 99.7272f (5, vss);
    penh w=29.6u l=1.6u (8, 5, 6);
    penh w=29.6u l=1.6u (C, 7, vdd);
    nenh w=23.2u l=1.6u (C, Y3, vss);
    nenh w=23.2u l=1.6u (12, Y7, vss);
    cap 96.6048f (6, vss);
    penh w=29.6u l=1.6u (12, 6, Y7);
    cap 96.6048f (7, vss);
    penh w=29.6u l=1.6u (8, 7, 9);
    nenh w=23.2u l=1.6u (8, Y3, vss);
    cap 306.4407f (8, vss);
    cap 180.2249f (Y7, vss);
    cap 96.6048f (9, vss);
    penh w=29.6u l=1.6u (12, 9, Y3);
    nenh w=23.2u l=1.6u (12, Y3, vss);
    nenh w=23.2u l=1.6u (A, 12, vss);
    penh w=29.6u l=1.6u (A, 12, vdd);
    cap 182.0145f (Y3, vss);
    nenh w=23.2u l=1.6u (15, Y5, vss);
    penh w=29.6u l=1.6u (15, 10, vdd);
    penh w=29.6u l=1.6u (C, 11, vdd);
    nenh w=23.2u l=1.6u (C, Y1, vss);
    nenh w=23.2u l=1.6u (12, Y5, vss);
    cap 99.02096f (10, vss);
    penh w=29.6u l=1.6u (12, 10, 13);
    cap 99.7272f (11, vss);
    penh w=29.6u l=1.6u (12, 11, 14);
    nenh w=23.2u l=1.6u (12, Y1, vss);
    cap 262.2697f (12, vss);
    nenh w=23.2u l=1.6u (B, Y5, vss);
    cap 96.6048f (13, vss);
    penh w=29.6u l=1.6u (B, 13, Y5);
    cap 96.6048f (14, vss);
    penh w=29.6u l=1.6u (B, 14, Y1);
    nenh w=23.2u l=1.6u (B, Y1, vss);
    cap 180.2249f (Y5, vss);
    cap 180.2249f (Y1, vss);
    nenh w=23.2u l=1.6u (15, Y4, vss);
    penh w=29.6u l=1.6u (15, 16, vdd);
    cap 280.3127f (15, vss);
    penh w=29.6u l=1.6u (C, 17, vdd);
    nenh w=23.2u l=1.6u (C, Y0, vss);
    cap 133.9119f (C, vss);
    nenh w=23.2u l=1.6u (B, Y4, vss);
    cap 97.24288f (16, vss);
    penh w=29.6u l=1.6u (B, 16, 18);
    cap 100.2344f (17, vss);
    penh w=29.6u l=1.6u (B, 17, 19);
    nenh w=23.2u l=1.6u (B, Y0, vss);
    cap 188.5443f (B, vss);
    nenh w=23.2u l=1.6u (A, Y4, vss);
    cap 98.048f (18, vss);
    penh w=29.6u l=1.6u (A, 18, Y4);
    cap 94.62592f (19, vss);
    penh w=29.6u l=1.6u (A, 19, Y0);
    nenh w=23.2u l=1.6u (A, Y0, vss);
    cap 195.6775f (A, vss);
    cap 171.6119f (Y4, vss);
    cap 171.6119f (Y0, vss);
    cap 957.6684f (vss, vss);
    cap 1.115111p (vdd, vss);
}

network Dfn10 (terminal D, CK, Q, vss, vdd)
{
    nenh w=23.2u l=1.6u (CK, 53, vss);
    penh w=29.6u l=1.6u (CK, 53, vdd);
    nenh w=23.2u l=1.6u (53, 54, vss);
    penh w=29.6u l=1.6u (53, 54, vdd);
    nenh w=23.2u l=1.6u (53, 17, vss);
    penh w=29.6u l=1.6u (CK, 18, vdd);
    cap 63.29385f (CK, vss);
    cap 38.94688f (17, vss);
    nenh w=23.2u l=1.6u (D, 17, 33);
    cap 100.3146f (18, vss);
    penh w=29.6u l=1.6u (D, 18, 33);
    cap 39.09853f (D, vss);
    nenh w=23.2u l=1.6u (54, 27, 33);
    penh w=29.6u l=1.6u (53, 28, 33);
    cap 34.79616f (27, vss);
    nenh w=23.2u l=1.6u (42, 27, vss);
    cap 95.43168f (28, vss);
    penh w=29.6u l=1.6u (42, 28, vdd);
    nenh w=23.2u l=1.6u (33, 42, vss);
    penh w=29.6u l=1.6u (33, 42, vdd);
    cap 168.5316f (33, vss);
    nenh w=23.2u l=1.6u (42, 47, vss);
    penh w=29.6u l=1.6u (42, 48, vdd);
    cap 177.3977f (42, vss);
    cap 34.26112f (47, vss);
    nenh w=23.2u l=1.6u (54, 47, 63);
    cap 98.90672f (48, vss);
    penh w=29.6u l=1.6u (53, 48, 63);
    nenh w=23.2u l=1.6u (53, 63, 73);
    cap 279.5292f (53, vss);
    penh w=29.6u l=1.6u (54, 63, 73);
    cap 209.5428f (54, vss);
    nenh w=23.2u l=1.6u (68, 73, vss);
    penh w=29.6u l=1.6u (68, 73, vdd);
    nenh w=23.2u l=1.6u (63, 68, vss);
    penh w=29.6u l=1.6u (63, 68, vdd);
    cap 170.2069f (63, vss);
    cap 158.0071f (68, vss);
    nenh w=23.2u l=1.6u (73, Q, vss);
    penh w=29.6u l=1.6u (73, Q, vdd);
    cap 166.0615f (73, vss);
    cap 133.7273f (Q, vss);
    cap 336.6503f (vss, vss);
    cap 698.0887f (vdd, vss);
}

network Dfr11 (terminal D, R, CK, Q, vss, vdd)
{
    nenh w=23.2u l=1.6u (D, 5, vss);
    penh w=29.6u l=1.6u (D, 5, vdd);
    cap 15.69586f (D, vss);
    nenh w=23.2u l=1.6u (5, 37, vss);
    penh w=29.6u l=1.6u (5, 10, vdd);
    cap 191.8417f (5, vss);
    nenh w=23.2u l=1.6u (R, 37, vss);
    cap 92.544f (10, vss);
    penh w=29.6u l=1.6u (R, 10, 37);
    cap 15.1733f (R, vss);
    nenh w=23.2u l=1.6u (CK, 72, vss);
    penh w=29.6u l=1.6u (CK, 72, vdd);
    nenh w=23.2u l=1.6u (72, 73, vss);
    penh w=29.6u l=1.6u (72, 73, vdd);
    nenh w=23.2u l=1.6u (72, 35, vss);
    penh w=29.6u l=1.6u (CK, 36, vdd);
    cap 41.17975f (CK, vss);
    cap 39.36768f (35, vss);
    nenh w=23.2u l=1.6u (37, 35, 52);
    cap 100.3146f (36, vss);
    penh w=29.6u l=1.6u (37, 36, 52);
    cap 187.8628f (37, vss);
    nenh w=23.2u l=1.6u (73, 46, 52);
    penh w=29.6u l=1.6u (72, 47, 52);
    cap 34.79616f (46, vss);
    nenh w=23.2u l=1.6u (61, 46, vss);
    cap 95.43168f (47, vss);
    penh w=29.6u l=1.6u (61, 47, vdd);
    nenh w=23.2u l=1.6u (52, 61, vss);
    penh w=29.6u l=1.6u (52, 61, vdd);
    cap 168.5316f (52, vss);
    nenh w=23.2u l=1.6u (61, 66, vss);
    penh w=29.6u l=1.6u (61, 67, vdd);
    cap 177.3977f (61, vss);
    cap 34.26112f (66, vss);
    nenh w=23.2u l=1.6u (73, 66, 82);
    cap 98.90672f (67, vss);
    penh w=29.6u l=1.6u (72, 67, 82);
    nenh w=23.2u l=1.6u (72, 82, 92);
    cap 239.4598f (72, vss);
    penh w=29.6u l=1.6u (73, 82, 92);
    cap 210.8671f (73, vss);
    nenh w=23.2u l=1.6u (87, 92, vss);
    penh w=29.6u l=1.6u (87, 92, vdd);
    nenh w=23.2u l=1.6u (82, 87, vss);
    penh w=29.6u l=1.6u (82, 87, vdd);
    cap 170.2069f (82, vss);
    cap 158.0071f (87, vss);
    nenh w=23.2u l=1.6u (92, Q, vss);
    penh w=29.6u l=1.6u (92, Q, vdd);
    cap 166.0615f (92, vss);
    cap 133.7273f (Q, vss);
    cap 436.1518f (vss, vss);
    cap 821.4392f (vdd, vss);
}

network Dfa11 (terminal D, R, CK, Q, vss, vdd)
{
    nenh w=23.2u l=1.6u (R, 63, vss);
    penh w=29.6u l=1.6u (R, 5, 63);
    nenh w=23.2u l=1.6u (CK, 63, vss);
    cap 97.49968f (5, vss);
    penh w=29.6u l=1.6u (CK, 5, vdd);
    nenh w=23.2u l=1.6u (63, 64, vss);
    penh w=29.6u l=1.6u (63, 64, vdd);
    nenh w=23.2u l=1.6u (63, 22, vss);
    penh w=29.6u l=1.6u (CK, 23, vdd);
    cap 63.45225f (CK, vss);
    cap 38.94688f (22, vss);
    nenh w=23.2u l=1.6u (D, 22, 38);
    cap 101.2736f (23, vss);
    penh w=29.6u l=1.6u (D, 23, 38);
    cap 40.12317f (D, vss);
    nenh w=23.2u l=1.6u (64, 32, 38);
    penh w=29.6u l=1.6u (63, 33, 38);
    cap 34.79616f (32, vss);
    nenh w=23.2u l=1.6u (52, 32, vss);
    cap 96.87552f (33, vss);
    penh w=29.6u l=1.6u (52, 33, vdd);
    nenh w=23.2u l=1.6u (38, 52, vss);
    penh w=29.6u l=1.6u (38, 43, vdd);
    cap 170.0132f (38, vss);
    nenh w=23.2u l=1.6u (R, 52, vss);
    cap 98.93216f (43, vss);
    penh w=29.6u l=1.6u (R, 43, 52);
    cap 70.9559f (R, vss);
    nenh w=23.2u l=1.6u (52, 57, vss);
    penh w=29.6u l=1.6u (52, 58, vdd);
    cap 180.0436f (52, vss);
    cap 34.26112f (57, vss);
    nenh w=23.2u l=1.6u (64, 57, 73);
    cap 98.90672f (58, vss);
    penh w=29.6u l=1.6u (63, 58, 73);
    nenh w=23.2u l=1.6u (63, 73, 83);
    cap 278.2286f (63, vss);
    penh w=29.6u l=1.6u (64, 73, 83);
    cap 212.9531f (64, vss);
    nenh w=23.2u l=1.6u (78, 83, vss);
    penh w=29.6u l=1.6u (78, 83, vdd);
    nenh w=23.2u l=1.6u (73, 78, vss);
    penh w=29.6u l=1.6u (73, 78, vdd);
    cap 170.2069f (73, vss);
    cap 158.0071f (78, vss);
    nenh w=23.2u l=1.6u (83, Q, vss);
    penh w=29.6u l=1.6u (83, Q, vdd);
    cap 166.0615f (83, vss);
    cap 133.7273f (Q, vss);
    cap 433.3065f (vss, vss);
    cap 700.5383f (vdd, vss);
}

network Ex210 (terminal A, B, Y, vss, vdd)
{
    nenh w=23.2u l=1.6u (A, 5, vss);
    penh w=29.6u l=1.6u (A, 5, vdd);
    nenh w=23.2u l=1.6u (5, 10, vss);
    penh w=29.6u l=1.6u (5, 11, vdd);
    cap 192.2575f (5, vss);
    cap 35.70496f (10, vss);
    nenh w=23.2u l=1.6u (30, 10, Y);
    cap 98.92064f (11, vss);
    penh w=29.6u l=1.6u (B, 11, Y);
    nenh w=23.2u l=1.6u (A, Y, 20);
    cap 150.3804f (Y, vss);
    penh w=29.6u l=1.6u (A, Y, 21);
    cap 40.57752f (A, vss);
    cap 33.35232f (20, vss);
    nenh w=23.2u l=1.6u (B, 20, vss);
    cap 94.03872f (21, vss);
    penh w=29.6u l=1.6u (30, 21, vdd);
    nenh w=23.2u l=1.6u (B, 30, vss);
    penh w=29.6u l=1.6u (B, 30, vdd);
    cap 41.55006f (B, vss);
    cap 166.2573f (30, vss);
    cap 115.684f (vss, vss);
    cap 232.7577f (vdd, vss);
}

network Iv110 (terminal A, Y, vss, vdd)
{
    nenh w=23.2u l=1.6u (A, Y, vss);
    penh w=29.6u l=1.6u (A, Y, vdd);
    cap 290f (Y, vss);
}

network Mu111 (terminal A, B, S, Y, vss, vdd)
{
    nenh w=23.2u l=1.6u (S, 15, vss);
    penh w=29.6u l=1.6u (S, 15, vdd);
    nenh w=23.2u l=1.6u (S, 9, vss);
    penh w=29.6u l=1.6u (15, 10, vdd);
    cap 37.63312f (9, vss);
    nenh w=23.2u l=1.6u (B, 9, 26);
    cap 99.00704f (10, vss);
    penh w=29.6u l=1.6u (B, 10, 26);
    cap 37.62189f (B, vss);
    nenh w=23.2u l=1.6u (15, 20, 26);
    cap 202.3233f (15, vss);
    penh w=29.6u l=1.6u (S, 21, 26);
    cap 47.05848f (S, vss);
    cap 33.62304f (20, vss);
    nenh w=23.2u l=1.6u (A, 20, vss);
    cap 93.98784f (21, vss);
    penh w=29.6u l=1.6u (A, 21, vdd);
    cap 15.69586f (A, vss);
    nenh w=23.2u l=1.6u (26, Y, vss);
    penh w=29.6u l=1.6u (26, Y, vdd);
    cap 158.737f (26, vss);
    cap 133.7273f (Y, vss);
    cap 111.1091f (vss, vss);
    cap 233.1681f (vdd, vss);
}

network Mu210 (terminal S1, S2, A, B, C, D, Y, vss, vdd)
{
    nenh w=23.2u l=1.6u (S1, 41, vss);
    penh w=29.6u l=1.6u (S1, 41, vdd);
    cap 120.984f (1, vss);
    penh w=29.6u l=1.6u (2, 1, 5);
    cap 5.06697f (2, vss);
    cap 43.3936f (3, vss);
    nenh w=23.2u l=1.6u (4, 3, 7);
    cap 5.06697f (4, vss);
    nenh w=23.2u l=1.6u (S1, 9, vss);
    penh w=29.6u l=1.6u (41, 10, vdd);
    cap 92.544f (5, vss);
    penh w=29.6u l=1.6u (6, 5, 11);
    cap 5.06697f (6, vss);
    cap 32.1792f (7, vss);
    nenh w=23.2u l=1.6u (8, 7, 13);
    cap 5.06697f (8, vss);
    cap 37.73344f (9, vss);
    nenh w=23.2u l=1.6u (D, 9, 50);
    cap 99.00704f (10, vss);
    penh w=29.6u l=1.6u (D, 10, 51);
    cap 37.99844f (D, vss);
    cap 92.544f (11, vss);
    penh w=29.6u l=1.6u (12, 11, 15);
    cap 5.06697f (12, vss);
    cap 32.1792f (13, vss);
    nenh w=23.2u l=1.6u (14, 13, 17);
    cap 5.06697f (14, vss);
    nenh w=23.2u l=1.6u (C, 19, 50);
    penh w=29.6u l=1.6u (C, 20, 51);
    cap 40.02877f (C, vss);
    cap 92.544f (15, vss);
    penh w=29.6u l=1.6u (16, 15, 21);
    cap 5.06697f (16, vss);
    cap 32.1792f (17, vss);
    nenh w=23.2u l=1.6u (18, 17, 23);
    cap 5.06697f (18, vss);
    cap 35.06688f (19, vss);
    nenh w=23.2u l=1.6u (41, 19, vss);
    cap 95.43168f (20, vss);
    penh w=29.6u l=1.6u (S1, 20, vdd);
    cap 92.544f (21, vss);
    penh w=29.6u l=1.6u (22, 21, 25);
    cap 5.06697f (22, vss);
    cap 32.1792f (23, vss);
    nenh w=23.2u l=1.6u (24, 23, 27);
    cap 5.06697f (24, vss);
    nenh w=23.2u l=1.6u (41, 29, vss);
    penh w=29.6u l=1.6u (S1, 30, vdd);
    cap 92.544f (25, vss);
    penh w=29.6u l=1.6u (26, 25, 31);
    cap 5.06697f (26, vss);
    cap 32.1792f (27, vss);
    nenh w=23.2u l=1.6u (28, 27, 33);
    cap 5.06697f (28, vss);
    cap 37.73344f (29, vss);
    nenh w=23.2u l=1.6u (A, 29, 60);
    cap 99.96608f (30, vss);
    penh w=29.6u l=1.6u (A, 30, 61);
    cap 39.02308f (A, vss);
    cap 92.544f (31, vss);
    penh w=29.6u l=1.6u (32, 31, 35);
    cap 5.06697f (32, vss);
    cap 32.1792f (33, vss);
    nenh w=23.2u l=1.6u (34, 33, 37);
    cap 5.06697f (34, vss);
    nenh w=23.2u l=1.6u (B, 39, 60);
    penh w=29.6u l=1.6u (B, 40, 61);
    cap 41.05341f (B, vss);
    cap 92.544f (35, vss);
    penh w=29.6u l=1.6u (36, 35, 42);
    cap 5.06697f (36, vss);
    cap 32.1792f (37, vss);
    nenh w=23.2u l=1.6u (38, 37, 44);
    cap 5.06697f (38, vss);
    cap 35.06688f (39, vss);
    nenh w=23.2u l=1.6u (S1, 39, vss);
    cap 67.76492f (S1, vss);
    cap 96.87552f (40, vss);
    penh w=29.6u l=1.6u (41, 40, vdd);
    cap 246.1352f (41, vss);
    cap 92.544f (42, vss);
    penh w=29.6u l=1.6u (43, 42, 46);
    cap 5.06697f (43, vss);
    cap 32.1792f (44, vss);
    nenh w=23.2u l=1.6u (45, 44, 48);
    cap 5.06697f (45, vss);
    nenh w=23.2u l=1.6u (vss, 50, vss);
    penh w=29.6u l=1.6u (vdd, 51, vdd);
    cap 92.544f (46, vss);
    penh w=29.6u l=1.6u (47, 46, 52);
    cap 5.06697f (47, vss);
    cap 32.1792f (48, vss);
    nenh w=23.2u l=1.6u (49, 48, 54);
    cap 5.06697f (49, vss);
    cap 97.556f (50, vss);
    nenh w=23.2u l=1.6u (S2, 50, 66);
    cap 220.2174f (51, vss);
    penh w=29.6u l=1.6u (75, 51, 66);
    cap 92.544f (52, vss);
    penh w=29.6u l=1.6u (53, 52, 56);
    cap 5.06697f (53, vss);
    cap 32.1792f (54, vss);
    nenh w=23.2u l=1.6u (55, 54, 58);
    cap 5.06697f (55, vss);
    nenh w=23.2u l=1.6u (75, 60, 66);
    penh w=29.6u l=1.6u (S2, 61, 66);
    cap 92.544f (56, vss);
    penh w=29.6u l=1.6u (57, 56, 62);
    cap 5.06697f (57, vss);
    cap 32.1792f (58, vss);
    nenh w=23.2u l=1.6u (59, 58, 64);
    cap 5.06697f (59, vss);
    cap 88.40032f (60, vss);
    nenh w=23.2u l=1.6u (vss, 60, Y);
    cap 210.8974f (61, vss);
    penh w=29.6u l=1.6u (vdd, 61, Y);
    cap 92.544f (62, vss);
    penh w=29.6u l=1.6u (63, 62, 67);
    cap 5.06697f (63, vss);
    cap 32.1792f (64, vss);
    nenh w=23.2u l=1.6u (65, 64, 69);
    cap 5.06697f (65, vss);
    nenh w=23.2u l=1.6u (66, Y, vss);
    penh w=29.6u l=1.6u (66, Y, vdd);
    cap 162.2979f (66, vss);
    cap 92.544f (67, vss);
    penh w=29.6u l=1.6u (68, 67, 71);
    cap 5.06697f (68, vss);
    cap 32.1792f (69, vss);
    nenh w=23.2u l=1.6u (70, 69, 73);
    cap 5.06697f (70, vss);
    cap 150.2104f (Y, vss);
    nenh w=23.2u l=1.6u (S2, 75, vss);
    penh w=29.6u l=1.6u (S2, 75, vdd);
    cap 68.69353f (S2, vss);
    cap 92.544f (71, vss);
    penh w=29.6u l=1.6u (72, 71, 76);
    cap 5.06697f (72, vss);
    cap 32.1792f (73, vss);
    nenh w=23.2u l=1.6u (74, 73, 78);
    cap 5.06697f (74, vss);
    nenh w=23.2u l=1.6u (vss, 75, 80);
    cap 177.5085f (75, vss);
    penh w=29.6u l=1.6u (vdd, 75, 81);
    cap 92.544f (76, vss);
    penh w=29.6u l=1.6u (77, 76, 82);
    cap 5.06697f (77, vss);
    cap 32.1792f (78, vss);
    nenh w=23.2u l=1.6u (79, 78, 83);
    cap 5.06697f (79, vss);
    cap 239.3444f (vss, vss);
    cap 29.3152f (80, vss);
    cap 80.496f (81, vss);
    cap 80.496f (82, vss);
    cap 29.3152f (83, vss);
    cap 480.9339f (vdd, vss);
}

network Na210 (terminal A, B, Y, vss, vdd)
{
    nenh w=23.2u l=1.6u (A, 5, vss);
    nenh w=23.2u l=1.6u (B, 5, Y);
    penh w=29.6u l=1.6u (A, Y, vdd);
    penh w=29.6u l=1.6u (B, Y, vdd);
    cap 185f (Y, vss);
    cap 30f (5, vss);
}

network Na310 (terminal A, B, C, Y, vss, vdd)
{
    nenh w=23.2u l=1.6u (A, 5, vss);
    penh w=29.6u l=1.6u (A, Y, vdd);
    nenh w=23.2u l=1.6u (B, 5, 10);
    penh w=29.6u l=1.6u (B, Y, vdd);
    nenh w=23.2u l=1.6u (C, 10, Y);
    penh w=29.6u l=1.6u (C, Y, vdd);
    cap 320f (Y, vss);
    cap 35f (5, vss);
    cap 35f (10, vss);
}

network No210 (terminal A, B, Y, vss, vdd)
{
    nenh w=23.2u l=1.6u (A, Y, vss);
    nenh w=23.2u l=1.6u (B, Y, vss);
    penh w=29.6u l=1.6u (A, 5, vdd);
    penh w=29.6u l=1.6u (B, 5, Y);
    cap 90f (5, vss);
    cap 240f (Y, vss);
}

network No310 (terminal A, B, C, Y, vss, vdd)
{
    nenh w=23.2u l=1.6u (A, Y, vss);
    penh w=29.6u l=1.6u (A, 5, vdd);
    nenh w=23.2u l=1.6u (B, Y, vss);
    penh w=29.6u l=1.6u (B, 5, 10);
    nenh w=23.2u l=1.6u (C, Y, vss);
    penh w=29.6u l=1.6u (C, 10, Y);
    cap 100f (5, vss);
    cap 100f (10, vss);
    cap 285f (Y, vss);
}

