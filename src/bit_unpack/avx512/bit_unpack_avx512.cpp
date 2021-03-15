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

#ifdef __AVX512F__

#include "bit_unpack_avx512.hpp"

inline __m512i load_and_uncompress_vn(const __mmask64* _x_) {
    const __mmask64 x = _x_[0];
    const __m512i p0 = _mm512_loadu_si512( (__m512i*) one_array );
    const __m512i p1 = _mm512_loadu_si512( (__m512i*)mone_array );
    const __m512i p  = _mm512_mask_blend_epi8( x, p0, p1 );
    return p;   // ON RESSORT LA VALEUR -1 => +1
}

inline __m512i load_and_uncompress_msg(const __m128i* _ptr_, const __m512i c1, const __m512i c2) {
    const __mmask64* ptr = (__mmask64*)_ptr_;
    const __mmask64  x1  = ptr[0];
    const __mmask64  x2  = ptr[1];
    const __m512i    v0  = _mm512_setzero_si512();
    const __m512i    w1  = _mm512_mask_blend_epi8( x1, v0, c1 );
    const __m512i    w2  = _mm512_mask_blend_epi8( x2, v0, c2 );
    const __m512i    p   = _mm512_or_si512        (w1, w2);
    return p;   // ON RESSORT LA VALEUR -1 => +1
}
template < > inline __m512i load_and_uncompress_msg(const __m512i* ptr, const __m512i v1, const __m512i v2) {
    return _mm512_loadu_si512(ptr);
}

void bit_unpack_avx512(
              uint8_t* dst,
        const uint8_t* src,
        const int32_t length)
{

}

#endif //__avx2_2__
