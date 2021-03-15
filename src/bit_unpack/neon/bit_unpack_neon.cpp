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

#ifdef __SSE4_2__

#include "bit_unpack_neon.hpp"

uint32_t m_values[16] = {
        0x00000000, 0x00000001, 0x00000100, 0x00000101,
        0x00010000, 0x00010001, 0x00010100, 0x00010101,
        0x01000000, 0x01000001, 0x01000100, 0x01000101,
        0x01010000, 0x01010001, 0x01010100, 0x01010101
};

void bit_unpack_neon(uint8_t* dst, const uint8_t* src, const int32_t length)
{
    if( length%8 != 0 )
    {
        printf("(EE) The array length that have (length%%8 != 0) are not currently managed !");
        exit( EXIT_FAILURE );
    }

    const int32_t rounds = (length / sizeof(uint64_t));  // 64 bits car on fait 2 traitements 32 bits par iteration
//  const int32_t middle = sizeof(__m128i) * rounds;

    const uint8_t* ptr_i = (const uint8_t*)src;
    uint32_t*  ptr_o = (uint32_t*)dst;

#pragma loop unroll
    for(int32_t i = 0; i < rounds; i += 1)
    {
        const uint32_t a = ptr_i[i];
        (*ptr_o++) = m_values[ (a     ) & 0x0F ];
        (*ptr_o++) = m_values[ (a >> 4) & 0x0F ];
    }
}

#endif //__SSE4_2__
