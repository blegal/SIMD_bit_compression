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

extern const uint8_t mask1a[32];
extern const uint8_t mask2a[32];
extern const uint8_t one_array[64];
extern const uint8_t mone_array[64];


inline __m128i load_and_uncompress_vn(const int16_t* x) {
#ifndef __AVX512F__
    const auto mask2 = _mm_loadu_si128( (__m128i*)mask2a );
    const auto mask1 = _mm_loadu_si128( (__m128i*)mask1a );
    const auto ones  = _mm_loadu_si128( (__m128i*)one_array );
    const auto y = _mm_set1_epi32( *x );
    const auto z = _mm_shuffle_epi8(y, mask1);
    const auto u = _mm_and_si128   (z, mask2);
    const auto v = _mm_cmpeq_epi8  (u, _mm_setzero_si128());    // LES -1
    const auto w = _mm_xor_si128   (v, _mm_cmpeq_epi8(v,v));    // INVERSION DU MASQUE
    const auto b = _mm_and_si128   (v, ones );                 // On laisse passer les 1
    const auto p = _mm_or_si128    (w,    b );
    return p;   // ON RESSORT LA VALEUR -1 => +1
#else
    const auto zero  = _mm_setzero_si128( );
    const auto pone  = _mm_loadu_si128  ( (__m128i*)one_array );
    const auto mone  = _mm_cmpeq_epi8   ( zero, zero);
    const auto p     = _mm_mask_mov_epi8( pone, *x, mone );
    return p;   // ON RESSORT LA VALEUR -1 => +1
#endif
}
inline __m128i load_and_uncompress_msg(const int32_t* _ptr_, const __m128i c1, const __m128i c2) {
    const int16_t* ptr = (int16_t*)_ptr_;
#ifndef __AVX512F__
    const auto mask2   = _mm_loadu_si128( (__m128i*)mask2a );
    const auto mask1   = _mm_loadu_si128( (__m128i*)mask1a );
    const auto y1 = _mm_set1_epi32( ptr[0] );
    const auto y2 = _mm_set1_epi32( ptr[1] );
    const auto z1 = _mm_shuffle_epi8(y1, mask1);
    const auto z2 = _mm_shuffle_epi8(y2, mask1);
    const auto u1 = _mm_and_si128   (z1, mask2);
    const auto u2 = _mm_and_si128   (z2, mask2);
    const auto v1 = _mm_cmpeq_epi8  (u1, _mm_setzero_si128());    // Les FF correspondent au 1 (one)
    const auto v2 = _mm_cmpeq_epi8  (u2, _mm_setzero_si128());    // Les FF correspondent au 1 (minus one)
    const auto w1 = _mm_andnot_si128   (v1, c1);    // INVERSION DU MASQUE
    const auto w2 = _mm_andnot_si128   (v2, c2);    // INVERSION DU MASQUE
    const auto p  = _mm_or_si128    (w1, w2);
    return p;   // ON RESSORT LA VALEUR -1 => +1
#else
    const auto zero  = _mm_setzero_si128( );
    const auto w1    = _mm_mask_mov_epi8( zero, ptr[0], c1 );
    const auto w2    = _mm_mask_mov_epi8( zero, ptr[1], c2 );
    const auto p     = _mm_or_si128    (w1, w2);
    return p;   // ON RESSORT LA VALEUR -1 => +1
#endif
}

void bit_unpack_sse4(uint8_t* dst, const uint8_t* src, const int32_t length)
{

}

#endif //__SSE4_2__
