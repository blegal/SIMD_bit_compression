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
#include "bit_pack_sse4.hpp"

inline void compress_and_store_msg(int32_t* _ptr_, const __m128i x, const __m128i v1, const __m128i v2) {
    int16_t* ptr = (int16_t*)_ptr_;
    const auto r = _mm_cmpeq_epi8(x, v1);
    const auto s = _mm_cmpeq_epi8(x, v2);
    ptr[0] = _mm_movemask_epi8( r );
    ptr[1] = _mm_movemask_epi8( s );
}

inline void compress_and_store_vn(int16_t* ptr, const __m128i x) {
    const auto r = _mm_movemask_epi8(x);
    (*ptr) = r;
}

void bit_pack_sse4(uint8_t* __restrict dst, const uint8_t* __restrict src, const int32_t length)
{
    if( length%8 != 0 )
    {
        printf("(EE) The array length that have (length%%8 != 0) are not currently managed !");
        exit( EXIT_FAILURE );
    }

    const int32_t rounds = (length / sizeof(__m128i));
    const int32_t middle = sizeof(__m128i) * rounds;
    const __m128i c_one  = _mm_set1_epi8( 1 );
    const __m128i*  ptr_i = (const __m128i*)src;
          uint16_t* ptr_o = (uint16_t*)dst;

#pragma loop unroll
    for(int32_t i = 0; i < rounds; i += 1)
    {
        const __m128i  v = _mm_loadu_si128(ptr_i + i);
        const __m128i  w = _mm_cmpeq_epi8(v, c_one);
        const uint32_t x = _mm_movemask_epi8(w);
        ptr_o[i]         = x;
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

#endif //__SSE4_2__
