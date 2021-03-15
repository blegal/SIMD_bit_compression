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
#include "bit_pack_avx2.hpp"

inline void compress_and_store_vn(int32_t* ptr, const __m256i x) {
    const auto r = _mm256_movemask_epi8(x);
    (*ptr) = r;
}

inline void compress_and_store_msg(__mmask64* _ptr_, const __m256i x, const __m256i v1, const __m256i v2) {
    int32_t* ptr = (int32_t*)_ptr_;
    const auto r = _mm256_cmpeq_epi8(x, v1);
    const auto s = _mm256_cmpeq_epi8(x, v2);
    ptr[0] = _mm256_movemask_epi8( r );
    ptr[1] = _mm256_movemask_epi8( s );
}

void bit_pack_avx2(uint8_t* dst, const uint8_t* src, const int32_t length)
{
    if( length%8 != 0 )
    {
        printf("(EE) The array length that have (length%%8 != 0) are not currently managed !");
        exit( EXIT_FAILURE );
    }

    const int32_t rounds = (length / sizeof(__m256i));
    const int32_t middle = sizeof(__m256i) * rounds;
    const __m256i c_one  = _mm256_set1_epi8( 1 );
    const __m256i*  ptr_i = (const __m256i*)src;
    uint32_t* ptr_o = (uint32_t*)dst;

#pragma loop unroll
    for(int32_t i = 0; i < rounds; i += 1)
    {
        const __m256i  v = _mm256_loadu_si256(ptr_i + i);
        const __m256i  w = _mm256_cmpeq_epi8(v, c_one);
        const uint32_t x = _mm256_movemask_epi8(w);
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

#endif //__AVX2__
