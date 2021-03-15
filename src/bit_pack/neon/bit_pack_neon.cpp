/*
*	Optimized bit-packing and bit-unpacking functions - Copyright (c) 2021 Bertrand LE GAL
*
*  This software is provided 'as-is', without any express or
*  implied warranty. In no event will the authors be held
*  liable for any damages arising from the use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute
*  it freely, subject to the following restrictions:
*
*  1. The origin of this software must not be misrepresented;
*  you must not claim that you wrote the original software.
*  If you use this software in a product, an acknowledgment
*  in the product documentation would be appreciated but
*  is not required.
*
*  2. Altered source versions must be plainly marked as such,
*  and must not be misrepresented as being the original software.
*
*  3. This notice may not be removed or altered from any
*  source distribution.
*
*/


#include "bit_pack_neon.hpp"

// Use shifts to collect all of the sign bits.
// I'm not sure if this works on big endian, but big endian NEON is very
// rare.
int vmovmaskq_u8(uint8x16_t input)
{
    // Example input (half scale):
    // 0x89 FF 1D C0 00 10 99 33

    // Shift out everything but the sign bits
    // 0x01 01 00 01 00 00 01 00
    uint16x8_t high_bits = vreinterpretq_u16_u8(vshrq_n_u8(input, 7));

    // Merge the even lanes together with vsra. The '??' bytes are garbage.
    // vsri could also be used, but it is slightly slower on aarch64.
    // 0x??03 ??02 ??00 ??01
    uint32x4_t paired16 = vreinterpretq_u32_u16(vsraq_n_u16(high_bits, high_bits, 7));
    // Repeat with wider lanes.
    // 0x??????0B ??????04
    uint64x2_t paired32 = vreinterpretq_u64_u32(vsraq_n_u32(paired16, paired16, 14));
    // 0x??????????????4B
    uint8x16_t paired64 = vreinterpretq_u8_u64(vsraq_n_u64(paired32, paired32, 28));
    // Extract the low 8 bits from each lane and join.
    // 0x4B
    return vgetq_lane_u8(paired64, 0) | ((int)vgetq_lane_u8(paired64, 8) << 8);
}

void bit_pack_neon(uint8_t* dst, const uint8_t* src, const int32_t length)
{
    if( length%8 != 0 )
    {
        printf("(EE) The array length that have (length%%8 != 0) are not currently managed !");
        exit( EXIT_FAILURE );
    }

    const int32_t rounds     = (length / sizeof(uint8x16_t));
    const int32_t middle     = sizeof(uint8x16_t) * rounds;
    const uint8x16_t c_one   = vmovq_n_s8( 1 );
    const uint8x16_t*  ptr_i = (const uint8x16_t*)src;
    uint16_t* ptr_o = (uint16_t*)dst;

#pragma loop unroll
    for(int32_t i = 0; i < rounds; i += 1)
    {
        const uint8x16_t  v = vld1q_s32(ptr_i + i);
        const uint8x16_t  w = vceqq_s8 (v, c_one);
        const uint32_t    x = vmovmaskq_u8(w);
        ptr_o[i]            = x;
    }

    //
    // Le patch scalaire qui termine les calculs si on n'a pas tout fait
    // en vectoriel
    //

    for(int32_t i = middle; i < length; i += 8)
    {
        uint8_t v = src[i];
#pragma clang loop unroll(full)
        for( uint32_t q = 1; q < 8 ; q += 1 )
        {
            v = v | (src[i+q] << q);
        }
        dst[i/8] = v;
    }

}
