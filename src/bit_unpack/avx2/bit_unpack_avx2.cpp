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

#ifdef __AVX2__

#include "bit_unpack_avx2.hpp"

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


void bit_unpack_avx2(
        uint8_t* dst,
        const uint8_t* src,
        const int32_t length)
{
    if( length%8 != 0 )
    {
        printf("(EE) The array length that have (length%%8 != 0) are not currently managed !");
        exit( EXIT_FAILURE );
    }

    const int32_t rounds = (length / sizeof(__m256i));
    const int32_t middle = sizeof(__m256i) * rounds;

    const uint32_t* ptr_i = (const uint32_t*)src;
    __m256i*  ptr_o = (__m256i*)dst;

    const auto mask2 = _mm256_loadu_si256( (__m256i*)mask2a    );
    const auto mask1 = _mm256_loadu_si256( (__m256i*)mask1a    );
    const auto ones  = _mm256_loadu_si256( (__m256i*)one_array );
    const auto zeros = _mm256_setzero_si256();

#pragma loop unroll
    for(int32_t i = 0; i < rounds; i += 1)
    {
        const uint32_t a = ptr_i[i];
        const auto y = _mm256_set1_epi32  (a       );
        const auto z = _mm256_shuffle_epi8(y, mask1);
        const auto u = _mm256_and_si256   (z, mask2);
        const auto v = _mm256_cmpeq_epi8  (u, zeros);
        const auto b = _mm256_andnot_si256(v, ones );
        _mm256_storeu_si256(ptr_o + i, b);
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

#endif //__avx2_2__
