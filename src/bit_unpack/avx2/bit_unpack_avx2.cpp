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
static const uint8_t mone_array[64] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

inline __m256i load_and_uncompress_vn(const int32_t* x) {
#ifndef __AVX512F__
    const auto mask2 = _mm256_loadu_si256( (__m256i*)mask2a    );
    const auto mask1 = _mm256_loadu_si256( (__m256i*)mask1a    );
    const auto ones  = _mm256_loadu_si256( (__m256i*)one_array );
    const auto y = _mm256_set1_epi32( *x );
    const auto z = _mm256_shuffle_epi8(y, mask1);
    const auto u = _mm256_and_si256   (z, mask2);
    const auto v = _mm256_cmpeq_epi8  (u, _mm256_setzero_si256());
    const auto w = _mm256_xor_si256   (v, _mm256_cmpeq_epi8(v,v));
    const auto b = _mm256_and_si256   (v, ones );                 // On laisse passer les 1
    const auto p = _mm256_or_si256    (w,    b );
    return p;   // ON RESSORT LA VALEUR -1 => +1
#else
    const auto zero  = _mm256_setzero_si256( );
    const auto pone  = _mm256_loadu_si256  ( (__m256i*)one_array );
    const auto mone  = _mm256_cmpeq_epi8   ( zero, zero);
    const auto p     = _mm256_mask_mov_epi8( pone, *x, mone );
    return p;   // ON RESSORT LA VALEUR -1 => +1
#endif
}
//__m512i _mm512_mask_mov_epi8    (__m512i src, __mmask64 k, __m512i a)
//__m512i _mm512_maskz_mov_epi8   (__mmask64 k, __m512i a)
//__m512i _mm512_maskz_mov_epi8   (__mmask64 k, __m512i a)
//__m512i _mm512_maskz_loadu_epi8 (__mmask64 k, void const* mem_addr)

inline __m256i load_and_uncompress_vn(const __m256i* ptr) {
    const auto p = _mm256_loadu_si256( (__m256i*)ptr );
    return p;
}

inline __m256i load_and_uncompress_msg(const __mmask64* _ptr_, const __m256i c1, const __m256i c2) {
    const int32_t* ptr = (int32_t*)_ptr_;
#ifndef __AVX512F__
    const auto mask2 = _mm256_loadu_si256( (__m256i*)mask2a );
    const auto mask1 = _mm256_loadu_si256( (__m256i*)mask1a );
    const auto y1    = _mm256_set1_epi32( ptr[0] );
    const auto y2    = _mm256_set1_epi32( ptr[1] );
    const auto z1    = _mm256_shuffle_epi8(y1, mask1);
    const auto z2    = _mm256_shuffle_epi8(y2, mask1);
    const auto u1    = _mm256_and_si256   (z1, mask2);
    const auto u2    = _mm256_and_si256   (z2, mask2);
    const auto v1    = _mm256_cmpeq_epi8  (u1, _mm256_setzero_si256());    // LES -1 => Les FF correspondent au 1
    const auto v2    = _mm256_cmpeq_epi8  (u2, _mm256_setzero_si256());    // LES -1 => Les FF correspondent au 1
    const auto w1    = _mm256_andnot_si256(v1, c1);    // INVERSION DU MASQUE
    const auto w2    = _mm256_andnot_si256(v2, c2);    // INVERSION DU MASQUE
    const auto p     = _mm256_or_si256    (w1, w2);
    return p;   // ON RESSORT LA VALEUR -1 => +1
#else
    const auto zero  = _mm256_setzero_si256( );
    const auto w1    = _mm256_mask_mov_epi8( zero, ptr[0], c1 );
    const auto w2    = _mm256_mask_mov_epi8( zero, ptr[1], c2 );
    const auto p     = _mm256_or_si256    (w1, w2);
    return p;   // ON RESSORT LA VALEUR -1 => +1
#endif

}

void bit_unpack_avx2(
        uint8_t* dst,
        const uint8_t* src,
        const int32_t length)
{

}

#endif //__avx2_2__
