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

#ifndef _bit_pack_avx512_avx512_
#define _bit_pack_avx512_avx512_
#ifdef __AVX512F__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern void bit_pack_avx512_avx512(uint8_t* dst, const uint8_t* src, const int32_t length);

#endif //__AVX512F__
#endif
    