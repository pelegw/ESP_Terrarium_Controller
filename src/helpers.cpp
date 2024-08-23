#include "helpers.h"
#include <stdint.h>

unsigned int f_2uint_int1(float float_number)
{ // split the float and return first unsigned integer

    union f_2uint
    {
        float f;
        uint16_t i[2];
    };

    union f_2uint f_number;
    f_number.f = float_number;

    return f_number.i[0];
}

unsigned int f_2uint_int2(float float_number)
{ // split the float and return first unsigned integer

    union f_2uint
    {
        float f;
        uint16_t i[2];
    };

    union f_2uint f_number;
    f_number.f = float_number;

    return f_number.i[1];
}