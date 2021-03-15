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

#if 0
    __m128i zero = _mm_setzero_si128();
    __m128i one0 = _mm_setr_epi8(  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15);
    __m128i one1 = _mm_setr_epi8( 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31);
    __m128i one2 = _mm_setr_epi8( 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47);
    __m128i one3 = _mm_setr_epi8( 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63);

    printf("one0 = "); uint8_t_show(one0);
    printf("one1 = "); uint8_t_show(one1);
    printf("one2 = "); uint8_t_show(one2);
    printf("one3 = "); uint8_t_show(one3);

    const __m128i two0   = _mm_unpacklo_epi8(one0, one1);
    const __m128i two1   = _mm_unpackhi_epi8(one0, one1);
    const __m128i two2   = _mm_unpacklo_epi8(one2, one3);
    const __m128i two3   = _mm_unpackhi_epi8(one2, one3);

    printf("\n");
    printf("two0 = "); uint8_t_show(two0);
    printf("two1 = "); uint8_t_show(two1);
    printf("two2 = "); uint8_t_show(two2);
    printf("two3 = "); uint8_t_show(two3);

    const __m128i t_0_1 = _mm_unpacklo_epi8( two0,  two1);
    const __m128i t_0_2 = _mm_unpackhi_epi8( two0,  two1);
    const __m128i t_0_3 = _mm_unpacklo_epi8( two2,  two3);
    const __m128i t_0_4 = _mm_unpackhi_epi8( two2,  two3);

    printf("\n");
    printf("t_0_1 = "); uint8_t_show(t_0_1);
    printf("t_0_2 = "); uint8_t_show(t_0_2);
    printf("t_0_3 = "); uint8_t_show(t_0_3);
    printf("t_0_4 = "); uint8_t_show(t_0_4);

    const __m128i t_1_1 = _mm_unpacklo_epi8(t_0_1, t_0_3);
    const __m128i t_1_2 = _mm_unpackhi_epi8(t_0_1, t_0_3);
    const __m128i t_1_3 = _mm_unpacklo_epi8(t_0_2, t_0_4);
    const __m128i t_1_4 = _mm_unpackhi_epi8(t_0_2, t_0_4);

    printf("\n");
    printf("t_1_1 = "); uint8_t_show(t_1_1);
    printf("t_1_2 = "); uint8_t_show(t_1_2);
    printf("t_1_3 = "); uint8_t_show(t_1_3);
    printf("t_1_4 = "); uint8_t_show(t_1_4);

    const __m128i t_2_1 = _mm_unpacklo_epi8(t_1_1, t_1_3);
    const __m128i t_2_2 = _mm_unpackhi_epi8(t_1_1, t_1_3);
    const __m128i t_2_3 = _mm_unpacklo_epi8(t_1_2, t_1_4);
    const __m128i t_2_4 = _mm_unpackhi_epi8(t_1_2, t_1_4);

    __m128i move = _mm_setr_epi8(15, 14, 11, 10, 7, 6, 3, 2, 13, 12, 9, 8, 5, 4, 1, 0);
//    __m128i _mm_shuffle_epi8 (__m128i a, __m128i b)

    printf("\n");
    printf("t_2_1 = "); uint8_t_show(t_2_1);
    printf("t_2_2 = "); uint8_t_show(t_2_2);
    printf("t_2_3 = "); uint8_t_show(t_2_3);
    printf("t_2_4 = "); uint8_t_show(t_2_4);

    const __m128i t_3_1 = _mm_shuffle_epi8(t_2_1, move);
    const __m128i t_3_2 = _mm_shuffle_epi8(t_2_2, move);
    const __m128i t_3_3 = _mm_shuffle_epi8(t_2_3, move);
    const __m128i t_3_4 = _mm_shuffle_epi8(t_2_4, move);

    printf("\n");
    printf("t_3_1 = "); uint8_t_show(t_3_1);
    printf("t_3_2 = "); uint8_t_show(t_3_2);
    printf("t_3_3 = "); uint8_t_show(t_3_3);
    printf("t_3_4 = "); uint8_t_show(t_3_4);
    exit( EXIT_SUCCESS );
#endif

#if 0
    const int32_t N = 512;
    uint8_t din[N], dtm[N], dou[N];
    for(int32_t i = 0; i < N; i += 1)
    {
        din[i] = i;
        dtm[i] = 0;
        dou[i] = 0;
    }

    uint8_t_show( din, N );
    printf("\n");
       bit_pack_avx2((__m256i*) dtm, (const __m256i*) din, N/16);
    bit_unpack_avx2((__m256i*) dou, (const __m256i*) dtm, N/16);
//    deinterleave_x86_frames((int8_t*)dou, (const int8_t*)din, N/4, 4);
    uint8_t_show( dtm, N );
    printf("\n");
    uint8_t_show( dou, N );
    printf("\n");

//    const __m128i four0 = _mm_unpacklo_epi16(three0, three2);
//    const __m128i four1 = _mm_unpackhi_epi16(three0, three2);
//    const __m128i four2 = _mm_unpacklo_epi16(three1, three3);
//    const __m128i four3 = _mm_unpackhi_epi16(three1, three3);

//    printf("\n");
//    printf("src1 = "); uint8_t_show(four0);
//    printf("src2 = "); uint8_t_show(four1);
//    printf("src3 = "); uint8_t_show(four2);
//    printf("src4 = "); uint8_t_show(four3);

    exit( EXIT_SUCCESS );
#endif

    const uint32_t v_begin = 4096;//128;
    const uint32_t v_end   = 4096;//65536;
    const uint32_t v_step  = 2;

    const  int32_t nTest   = 1024;// * 1024;

    for(uint32_t ll = v_begin; ll <= v_end; ll += v_step)
    {
        printf("+> testing functions [ll = %4d]\n", ll);
        uint8_t* i_tab  = new uint8_t[ll];
        uint8_t* o_x86  = new uint8_t[ll];
        uint8_t* d_x86  = new uint8_t[ll];
#ifdef __SSE4_2__
        uint8_t* o_sse  = new uint8_t[ll];
        uint8_t* d_sse  = new uint8_t[ll];
#endif
#ifdef __AVX2__
        uint8_t* o_avx  = new uint8_t[ll];
        uint8_t* d_avx  = new uint8_t[ll];
#endif
#ifdef __ARM_NEON__
        uint8_t* o_neon = new uint8_t[ll];
        uint8_t* d_neon = new uint8_t[ll];
#endif
        
        for (uint32_t i = 0; i < ll; i += 1) {
          i_tab[i] = rand()%256 - 128;
//            i_tab[i] = i % ( ll / 16);
        }

        //
        // Pour une taille de trame donnée, on va tester differentes valeurs
        // de parallelisme
        //
        const int32_t f_values[] = {1, 2, 3, 4, 5, 6, 7, 8, 16, 32, 64};
        const int32_t n_values   = sizeof(f_values) / sizeof(int32_t);
        for(int32_t fr = 0; fr < n_values; fr += 1)
        {
            const int32_t ff = f_values[fr];
            if( ll%ff != 0 )    // Si le nombre de données n'est pas un multiple de
            {
                continue;       // ff alors on ne peut pas entrelacer
            }

            //
            // On reinitialise les tableaux de sortie pour faire un check des resultats
            //
            for (uint32_t i = 0; i < ll; i += 1) {
                o_x86 [i] = 0;
                d_x86 [i] = 0;
#ifdef __SSE4_2__
                o_sse [i] = 0;
                d_sse [i] = 0;
#endif
#ifdef __AVX2__
                o_avx [i] = 0;
                d_avx [i] = 0;
#endif
#ifdef __ARM_NEON__
                o_neon[i] = 0;
                d_neon[i] = 0;
#endif
            }

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

            auto start_d_sse4 = std::chrono::system_clock::now();
            for(int32_t loop = 0; loop < nTest; loop += 1)
                bit_unpack_sse4(d_sse, o_sse, ll);
            auto stop_d_sse4 = std::chrono::system_clock::now();
#endif

            //
            // PROCEDURE DE TEST SUR LES CODES AVX2
            //

#ifdef __AVX2__
            auto start_i_avx2 = std::chrono::system_clock::now();
            for(int32_t loop = 0; loop < nTest; loop += 1)
                bit_pack_avx2(o_avx, i_tab, ll);
            auto stop_i_avx2 = std::chrono::system_clock::now();

            auto start_d_avx2 = std::chrono::system_clock::now();
            for(int32_t loop = 0; loop < nTest; loop += 1)
                bit_unpack_avx2(d_avx, o_avx, ll);
            auto stop_d_avx2 = std::chrono::system_clock::now();
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

            printf("    - [-x86-] %2dx bit_pack \033[32mOK\033[0m [%4d ns] - %2dx bit_unpack \033[32mOK\033[0m [%4d ns]\n", (int32_t)time_i_ref, (int32_t)time_d_ref);
            printf("    - [AUTOV] %2dx bit_pack \033[32mOK\033[0m [%4d ns] - %2dx bit_unpack \033[32mOK\033[0m [%4d ns]\n", (int32_t)time_i_x86, (int32_t)time_d_x86);

#ifdef __SSE4_2__
            const bool ok_i_sse = (memcmp(o_x86, o_sse, ll) == 0);
            const bool ok_d_sse = (memcmp(i_tab, d_sse, ll) == 0);
            if( ok_i_sse ){
                printf("    - [SSE-4] %2dx bit_pack \033[32mOK\033[0m [%4d ns] - ", (int32_t)time_i_sse4);
            }else{
                printf("    - [SSE-4] %2dx bit_pack \033[31mKO\033[0m [%4d ns] - ", (int32_t)time_i_sse4);
            }
            if( ok_d_sse ){
                printf("%2dx bit_unpack \033[32mOK\033[0m [%4d ns]\n", (int32_t)time_d_sse4);
            }else{
                printf("%2dx bit_unpack \033[31mKO\033[0m [%4d ns]\n", (int32_t)time_d_sse4);
            }
#endif
#ifdef __AVX2__
            const bool ok_i_avx = (memcmp(o_x86, o_avx, ll) == 0);
            const bool ok_d_avx = (memcmp(i_tab, d_avx, ll) == 0);
            if( ok_i_avx ){
                printf("    - [AVX-2] %2dx bit_pack \033[32mOK\033[0m [%4d ns] - ", (int32_t)time_i_avx2);
            }else{
                printf("    - [AVX-2] %2dx bit_pack \033[31mKO\033[0m [%4d ns] - ", (int32_t)time_i_avx2);
            }
            if( ok_d_avx ){
                printf("%2dx bit_unpack \033[32mOK\033[0m [%4d ns]\n", (int32_t)time_d_avx2);
            }else{
                printf("%2dx bit_unpack \033[31mKO\033[0m [%4d ns]\n", (int32_t)time_d_avx2);
            }
#endif
#ifdef __ARM_NEON__
            const bool ok_i_neon = (memcmp(o_x86, o_neon, ll) == 0);
            const bool ok_d_neon = (memcmp(i_tab, d_neon, ll) == 0);
            if( ok_i_neon ){
                printf("    - [-ARM-] %2dx bit_pack \033[32mOK\033[0m [%4d ns] - ", (int32_t)time_i_neon);
            }else{
                printf("    - [-ARM-] %2dx bit_pack \033[31mKO\033[0m [%4d ns] - ", (int32_t)time_i_neon);
            }
            if( ok_d_neon ){
                printf("%2dx bit_unpack \033[32mOK\033[0m [%4d ns]\n", (int32_t)time_d_neon);
            }else{
                printf("%2dx bit_unpack \033[31mKO\033[0m [%4d ns]\n", (int32_t)time_d_neon);
            }
#endif
            printf("\n");
        }
        
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
    