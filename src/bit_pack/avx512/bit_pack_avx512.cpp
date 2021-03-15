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

#include "bit_pack_avx512.hpp"
#ifdef __AVX512F__

template < > inline void compress_and_store_vn(__mmask64* ptr, const __m512i x) {
    const auto r = _mm512_movepi8_mask(x);
    (*ptr) = r;
}
template < > inline void compress_and_store_vn(__m512i* ptr, const __m512i x) {
    _mm512_store_si512(ptr, x);
}
template < > inline void compress_and_store_msg(__m128i* _ptr_, const __m512i x, const __m512i v1, const __m512i v2) {
    __mmask64* ptr = (__mmask64*)_ptr_;
    ptr[0] = _mm512_cmpeq_epi8_mask(x, v1);
    ptr[1] = _mm512_cmpeq_epi8_mask(x, v2);
}
template < > inline void compress_and_store_msg(__m512i* ptr, const __m512i x, const __m512i v1, const __m512i v2) {
    _mm512_store_si512(ptr, x);
}

void bit_pack_avx512(uint8_t* dst, const uint8_t* src, const int32_t length)
{

}

#endif //__AVX512F__
