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


#include "tools.hpp"

#include "./bit_pack/x86/bit_pack_x86.hpp"
#include "./bit_pack/sse4/bit_pack_sse4.hpp"
#include "./bit_pack/avx2/bit_pack_avx2.hpp"
#include "./bit_pack/neon/bit_pack_neon.hpp"

#include "./bit_unpack/x86/bit_unpack_x86.hpp"
#include "./bit_unpack/sse4/bit_unpack_sse4.hpp"
#include "./bit_unpack/avx2/bit_unpack_avx2.hpp"
#include "./bit_unpack/neon/bit_unpack_neon.hpp"

#include <cstring>
#include <chrono>

#ifndef __ARM_NEON__
#define __ARM_NEON__ // Mode emulation
#endif

void dump_uint8_bits(const uint8_t* bits, const int32_t ll)
{
    for(int32_t i = 0; i < ll; i += 1)
    {
             if( (i%128 == 0) && (i!=0) ) printf("\n");
        else if( (i%8   == 0) && (i!=0) ) printf(" ");
        printf("%d", bits[i]);
    }
    printf("\n");
}

void dump_bits(const uint8_t* bits, const int32_t ll)
{
    for(int32_t i = 0; i < ll; i += 8)
    {
        if( (i%128 == 0) && (i!=0) ) printf("\n");
        const uint32_t v = bits[i/8];
        for(int32_t q = 0; q < 8; q += 1)
            printf("%d", ((v >> q) & 0x01));
        printf(" ");
    }
    printf("\n");
}

#define _MODE_DEBUG_ false

int main(int argc, char* argv[])
{

#if defined (__APPLE__)
    printf("(II) Benchmarking the bit_pack/bit_unpack functions on MacOS\n");
#elif defined (__linux__)
    printf("(II) Benchmarking the bit_pack/bit_unpack functions on Linux\n");
#else
    printf("(II) Benchmarking the bit_pack/bit_unpack functions on a undefined platform\n");
#endif


#if defined (__clang__)
    printf("(II) Code compiled with LLVM (%d.%d.%d)\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined (__GNUC__)
    printf("(II) Code compiled with GCC (%d.%d.%d)\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
    printf("(II) Code compiled with UNKWON compiler\n");
#endif

    const uint32_t v_begin = 128;//128;
    const uint32_t v_end   = 65536;//65536;
    const uint32_t v_step  = 2;

    const  int32_t nTest   = (v_begin == v_end) ? (1024 * 1024) : (64 * 1024 * 1024);

    for(uint32_t ll = v_begin; ll <= v_end; ll *= v_step)
    {
        const int ll_bits = ((ll + 7) / 8); // Nombre d'octets nécessaires

        printf("+> testing functions [ll = %4d]\n", ll);
        uint8_t* i_tab  = new uint8_t[ll];
        uint8_t* o_x86  = new uint8_t[ll_bits];
        uint8_t* d_x86  = new uint8_t[ll];
#ifdef __SSE4_2__
        uint8_t* o_sse  = new uint8_t[ll_bits];
        uint8_t* d_sse  = new uint8_t[ll];
#endif
#ifdef __AVX2__
        uint8_t* o_avx  = new uint8_t[ll_bits];
        uint8_t* d_avx  = new uint8_t[ll];
#endif
#ifdef __ARM_NEON__
        uint8_t* o_neon = new uint8_t[ll_bits];
        uint8_t* d_neon = new uint8_t[ll];
#endif

        for (uint32_t i = 0; i < ll; i += 1) {
            i_tab[i] = rand()%2;
        }

        //
        // On reinitialise les tableaux de sortie pour faire un check des resultats
        //
        memset(o_x86, 0, ll_bits);
        memset(d_x86, 0, ll);

#ifdef __SSE4_2__
        memset(o_sse, 0, ll_bits);
        memset(d_sse, 0, ll);
#endif
#ifdef __AVX2__
        memset(o_avx, 0, ll_bits);
        memset(d_avx, 0, ll);
#endif
#ifdef __ARM_NEON__
        memset(o_neon, 0, ll_bits);
        memset(d_neon, 0, ll);
#endif

        //
        // PROCEDURE DE TEST SUR LES CODE NON-OPTIMISES
        //

        auto start_i_ref = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
            bit_pack_x86(o_x86, i_tab, ll);
        auto stop_i_ref = std::chrono::system_clock::now();
 
        auto start_d_ref = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
            bit_unpack_x86(d_x86, o_x86, ll);
        auto stop_d_ref = std::chrono::system_clock::now();

#if _MODE_DEBUG_ == true
        printf("\nVecteur de reference :\n");
        dump_uint8_bits(i_tab, ll);

        printf("\nVecteur encode :\n");
        dump_bits(o_x86, ll);

        printf("\nVecteur reconstruit :\n");
        dump_uint8_bits(d_x86, ll);
#endif

        //
        // PROCEDURE DE TEST SUR LES CODE AUTO-OPTIMISES
        //

        auto start_i_x86 = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
            bit_pack_x86(o_x86, i_tab, ll);
        auto stop_i_x86 = std::chrono::system_clock::now();

        auto start_d_x86 = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
            bit_unpack_x86(d_x86, o_x86, ll);
        auto stop_d_x86 = std::chrono::system_clock::now();

        //
        // PROCEDURE DE TEST SUR LES CODES SSE4
        //

#ifdef __SSE4_2__
        auto start_i_sse4 = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
            bit_pack_sse4(o_sse, i_tab, ll);
        auto stop_i_sse4 = std::chrono::system_clock::now();

#if _MODE_DEBUG_ == true
        printf("\nVecteur encode SSE4 :\n");
        dump_bits(o_sse, ll);
#endif

        auto start_d_sse4 = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
            bit_unpack_sse4(d_sse, o_sse, ll);
        auto stop_d_sse4 = std::chrono::system_clock::now();

#if _MODE_DEBUG_ == true
        printf("\nVecteur reconstruit SSE4 :\n");
        dump_uint8_bits(d_sse, ll);
#endif

#endif

        //
        // PROCEDURE DE TEST SUR LES CODES AVX2
        //

#ifdef __AVX2__
        auto start_i_avx2 = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
            bit_pack_avx2(o_avx, i_tab, ll);
        auto stop_i_avx2 = std::chrono::system_clock::now();

#if _MODE_DEBUG_ == true
        printf("\nVecteur encode AVX2 :\n");
        dump_bits(o_avx, ll);
#endif

        auto start_d_avx2 = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
            bit_unpack_avx2(d_avx, o_avx, ll);
        auto stop_d_avx2 = std::chrono::system_clock::now();

#if _MODE_DEBUG_ == true
        printf("\nVecteur reconstruit AVX2 :\n");
        dump_uint8_bits(d_avx, ll);
        exit( 0 );
#endif

#endif

        //
        // PROCEDURE DE TEST SUR LES CODES NEON
        //

#ifdef __ARM_NEON__
        auto start_i_neon = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
            bit_pack_neon(o_neon, i_tab, ll);
        auto stop_i_neon = std::chrono::system_clock::now();

        auto start_d_neon = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
            bit_unpack_neon(d_neon, o_neon, ll);
        auto stop_d_neon = std::chrono::system_clock::now();
#endif

        //
        // CALCUL DES TEMPS D'EXECUTION POUR LES STATISTIQUES
        //

        const uint64_t time_i_ref = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_ref - start_i_ref).count() / nTest;
        const uint64_t time_i_x86 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_x86 - start_i_x86).count() / nTest;
        const uint64_t time_d_ref = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_d_ref - start_d_ref).count() / nTest;
        const uint64_t time_d_x86 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_d_x86 - start_d_x86).count() / nTest;
#ifdef __SSE4_2__
        const uint64_t time_i_sse4 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_sse4 - start_i_sse4).count() / nTest;
        const uint64_t time_d_sse4 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_d_sse4 - start_d_sse4).count() / nTest;
#endif
#ifdef __AVX2__
        const uint64_t time_i_avx2 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_avx2 - start_i_avx2).count() / nTest;
        const uint64_t time_d_avx2 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_d_avx2 - start_d_avx2).count() / nTest;
#endif
#ifdef __ARM_NEON__
        const uint64_t time_i_neon = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_neon - start_i_neon).count() / nTest;
        const uint64_t time_d_neon = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_d_neon - start_d_neon).count() / nTest;
#endif

        //
        // On affiche les resultats de comparaison ainsi que les perfos temporelles
        //

        printf("    - [-x86-] bit_pack \033[32mOK\033[0m [%5d ns] - bit_unpack \033[32mOK\033[0m [%5d ns]\n", (int32_t)time_i_ref, (int32_t)time_d_ref);
        printf("    - [AUTOV] bit_pack \033[32mOK\033[0m [%5d ns] - bit_unpack \033[32mOK\033[0m [%5d ns]\n", (int32_t)time_i_x86, (int32_t)time_d_x86);

#ifdef __SSE4_2__
        const bool ok_i_sse = (memcmp(o_x86, o_sse, ll_bits) == 0);
        const bool ok_d_sse = (memcmp(i_tab, d_sse, ll     ) == 0);
        if( ok_i_sse ){
            printf("    - [SSE-4] bit_pack \033[32mOK\033[0m [%5d ns] - ", (int32_t)time_i_sse4);
        }else{
            printf("    - [SSE-4] bit_pack \033[31mKO\033[0m [%5d ns] - ", (int32_t)time_i_sse4);
        }
        if( ok_d_sse ){
            printf("bit_unpack \033[32mOK\033[0m [%5d ns]\n", (int32_t)time_d_sse4);
        }else{
            printf("bit_unpack \033[31mKO\033[0m [%5d ns]\n", (int32_t)time_d_sse4);
        }
#endif
#ifdef __AVX2__
        const bool ok_i_avx = (memcmp(o_x86, o_avx, ll_bits) == 0);
        const bool ok_d_avx = (memcmp(i_tab, d_avx, ll     ) == 0);
        if( ok_i_avx ){
            printf("    - [AVX-2] bit_pack \033[32mOK\033[0m [%5d ns] - ", (int32_t)time_i_avx2);
        }else{
            printf("    - [AVX-2] bit_pack \033[31mKO\033[0m [%5d ns] - ", (int32_t)time_i_avx2);
        }
        if( ok_d_avx ){
            printf("bit_unpack \033[32mOK\033[0m [%5d ns]\n", (int32_t)time_d_avx2);
        }else{
            printf("bit_unpack \033[31mKO\033[0m [%5d ns]\n", (int32_t)time_d_avx2);
        }
#endif
#ifdef __ARM_NEON__
        const bool ok_i_neon = (memcmp(o_x86, o_neon, ll_bits) == 0);
        const bool ok_d_neon = (memcmp(i_tab, d_neon, ll     ) == 0);
        if( ok_i_neon ){
            printf("    - [-ARM-] bit_pack \033[32mOK\033[0m [%5d ns] - ", (int32_t)time_i_neon);
        }else{
            printf("    - [-ARM-] bit_pack \033[31mKO\033[0m [%5d ns] - ", (int32_t)time_i_neon);
        }
        if( ok_d_neon ){
            printf("bit_unpack \033[32mOK\033[0m [%5d ns]\n", (int32_t)time_d_neon);
        }else{
            printf("bit_unpack \033[31mKO\033[0m [%5d ns]\n", (int32_t)time_d_neon);
        }
#endif
        printf("\n");
//        }

        delete[] i_tab;
        delete[] o_x86;
        delete[] d_x86;
#ifdef __SSE4_2__
        delete[] o_sse;
        delete[] d_sse;
#endif
#ifdef __AVX2__
        delete[] o_avx;
        delete[] d_avx;
#endif
#ifdef __ARM_NEON__
        delete[] o_neon;
        delete[] d_neon;
#endif
    }

    return EXIT_SUCCESS;
}
    
