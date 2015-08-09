#include "vultin.h"

#ifdef __GNUC__
#define clz(x) (__builtin_clzl(x) - (8 * sizeof(long) - 32))
#else
static uint8_t clz(uint32_t x)
{
    uint8_t result = 0;
    if (x == 0) return 32;
    while (!(x & 0xF0000000)) { result += 4; x <<= 4; }
    while (!(x & 0x80000000)) { result += 1; x <<= 1; }
    return result;
}
#endif

int32_t fix_div(int32_t a, int32_t b)
{
    if (b == 0) return 0;
    uint32_t remainder = (a >= 0) ? a : (-a);
    uint32_t divider = (b >= 0) ? b : (-b);
    uint32_t quotient = 0;
    int bit_pos = 17;
    if (divider & 0xFFF00000) {
        uint32_t shifted_div = ((divider >> 17) + 1);
        quotient = remainder / shifted_div;
        remainder -= ((uint64_t)quotient * divider) >> 17;
    }
    while (!(divider & 0xF) && bit_pos >= 4) {
        divider >>= 4;
        bit_pos -= 4;
    }
    while (remainder && bit_pos >= 0) {
        int shift = clz(remainder);
        if (shift > bit_pos) shift = bit_pos;
        remainder <<= shift;
        bit_pos -= shift;
        uint32_t div = remainder / divider;
        remainder = remainder % divider;
        quotient += div << bit_pos;
        remainder <<= 1;
        bit_pos--;
    }
    int32_t result = quotient >> 1;
    if ((a ^ b) & 0x80000000) {
        result = -result;
    }
    return result;
}

int32_t fix_exp(int32_t inValue) {
    if(inValue == 0        ) return 0x00010000;
    if(inValue == 0x00010000) return 178145;
    if(inValue >= 681391   ) return 0x7FFFFFFF;
    if(inValue <= -772243  ) return 0;
    // The power-series converges much faster on positive values
    // and exp(-x) = 1/exp(x).
    int neg = (inValue < 0);
    if (neg) inValue = -inValue;
    int32_t result = inValue + 0x00010000;
    int32_t term = inValue;
    uint_fast8_t i;
    for (i = 2; i < 30; i++) {
        term = fix_mul(term, fix_div(inValue, fix_from_int(i)));
        result += term;
        if ((term < 500) && ((i > 15) || (term < 20))) break;
    }
    if (neg) result = fix_div(0x00010000, result);
    return result;
}

int32_t fix_sin(int32_t inAngle)
{
    int32_t tempAngle = inAngle % (fix_pi << 1);

    if(tempAngle > fix_pi)
        tempAngle -= (fix_pi << 1);
    else if(tempAngle < -fix_pi)
        tempAngle += (fix_pi << 1);

    int32_t tempAngleSq = fix_mul(tempAngle, tempAngle);

    int32_t tempOut;
    tempOut = fix_mul(-13, tempAngleSq) + 546;
    tempOut = fix_mul(tempOut, tempAngleSq) - 10923;
    tempOut = fix_mul(tempOut, tempAngleSq) + 65536;
    tempOut = fix_mul(tempOut, tempAngle);

    return tempOut;
}
