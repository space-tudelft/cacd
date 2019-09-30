extern network hotelLogic(terminal s, freeze, q0, q1, q0new, q1new, vss, vdd)
extern network no310 (terminal A, B, C, Y, vss, vdd)
extern network dfr11 (terminal D, Q, R, CK, vss, vdd)
extern network iv110 (terminal A, Y, vss, vdd)

network hotel(terminal s1, s2, s3, freeze, light, ck, reset, vss, vdd)
{
{inst1} no310(s1, s2, s3, s_n, vss, vdd);
{inst2} iv110(s_n, s, vss, vdd);
{inst3} hotelLogic(s, freeze, q0, q1, q0new, q1new, vss, vdd);
{inst4} dfr11(q0new, q0, reset, ck, vss, vdd);
{inst5} dfr11(q1new, q1, reset, ck, vss, vdd);
net {q1, light};
}
