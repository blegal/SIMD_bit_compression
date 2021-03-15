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

#include "bit_pack_x86.hpp"

void bit_pack_x86(
              uint8_t* __restrict dst,
        const uint8_t* __restrict src,
        const int32_t length)
{
    if( length%8 != 0 )
    {
        printf("(EE) The array length that have (length%%8 != 0) are not currently managed !");
        exit( EXIT_FAILURE );
    }

    const uint8_t* ptr = src;
    const int32_t ll = length / 8;

    for(int32_t i = 0; i < ll; i += 1)
    {
        uint8_t v = (*ptr++);
#pragma clang loop unroll(full)
        for( uint32_t q = 1; q < 8 ; q += 1 )
        {
            v = v | ((*ptr) << q);
            ptr += 1;
        }
        dst[i] = v;
    }
}
