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

void bit_pack_sse4(uint8_t* dst, const uint8_t* src, const int32_t length)
{

}

#endif //__SSE4_2__
