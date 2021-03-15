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

#include "bit_unpack_sse4.hpp"

static const uint8_t mask1a[32] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01,
        0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02,
        0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03
};

static const uint8_t mask2a[32] = {
        0x01, 0x02, 0x04, 0x08,
        0x10, 0x20, 0x40, 0x80,
        0x01, 0x02, 0x04, 0x08,
        0x10, 0x20, 0x40, 0x80,
        0x01, 0x02, 0x04, 0x08,
        0x10, 0x20, 0x40, 0x80,
        0x01, 0x02, 0x04, 0x08,
        0x10, 0x20, 0x40, 0x80
};

static const uint8_t one_array[64] = {
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};


void bit_unpack_sse4(uint8_t* __restrict dst, const uint8_t* __restrict src, const int32_t length)
{
    if( length%8 != 0 )
    {
        printf("(EE) The array length that have (length%%8 != 0) are not currently managed !");
        exit( EXIT_FAILURE );
    }

    const int32_t rounds = (length / sizeof(__m128i));
    const int32_t middle = sizeof(__m128i) * rounds;

    const uint16_t* ptr_i = (const uint16_t*)src;
          __m128i*  ptr_o = (__m128i*)dst;

    const auto mask2 = _mm_loadu_si128( (__m128i*)mask2a    );
    const auto mask1 = _mm_loadu_si128( (__m128i*)mask1a    );
    const auto ones  = _mm_loadu_si128( (__m128i*)one_array );

#pragma loop unroll
    for(int32_t i = 0; i < rounds; i += 1)
    {
        const uint32_t a = ptr_i[i];
        const auto y = _mm_set1_epi32  (      a );
        const auto z = _mm_shuffle_epi8(y, mask1);
        const auto u = _mm_and_si128   (z, mask2);
        const auto v = _mm_cmpeq_epi8  (u, _mm_setzero_si128());
        const auto b = _mm_andnot_si128(v, ones );
        _mm_storeu_si128(ptr_o + i, b);
    }

    //
    // Le patch scalaire qui termine les calculs si on n'a pas tout fait
    // en vectoriel
    //
    for(int32_t i = middle; i < length; i += 8)
    {
        const uint32_t v = src[i/8];
#pragma clang loop unroll(full)
        for( uint32_t q = 0; q < 8 ; q += 1 )
        {
            dst[i+q] = (v >> q) & 0x01;
        }
    }
}

#endif //__SSE4_2__
