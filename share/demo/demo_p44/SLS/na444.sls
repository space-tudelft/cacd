extern network na210 (terminal A, B, Y, vss, vdd) 
network na444 (terminal A, B, C, D, Y, vss, vdd) 
{
   {na210_0} na210 (A, B, net_4, vss, vdd); 
   {na210_1} na210 (C, D, net_5, vss, vdd); 
   {na210_2} na210 (net_4, net_5, Y, vss, vdd); 
}
