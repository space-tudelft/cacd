
extern network na210 (terminal A, B, Y, vss, vdd)
extern network iv110 (terminal A, Y, vss, vdd)
network and2 (terminal A, B, Y, vss, vdd)
{
   {na210_0} na210 (A, B, is0, vss, vdd);
   {iv110_1} iv110 (is0, Y, vss, vdd);
}
