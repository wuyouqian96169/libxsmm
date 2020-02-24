/******************************************************************************
* Copyright (c) Intel Corporation - All rights reserved.                      *
* This file is part of the LIBXSMM library.                                   *
*                                                                             *
* For information on the license, see the LICENSE file.                       *
* Further information: https://github.com/hfp/libxsmm/                        *
* SPDX-License-Identifier: BSD-3-Clause                                       *
******************************************************************************/
/* Hans Pabst (Intel Corp.)
******************************************************************************/
#ifndef LIBXSMM_INTRINSICS_X86_H
#define LIBXSMM_INTRINSICS_X86_H

#include "libxsmm_cpuid.h"

/** https://github.com/intel/Immintrin-debug */
#if !defined(LIBXSMM_INTRINSICS_DEBUG) && 0
# define LIBXSMM_INTRINSICS_DEBUG
#endif
#if defined(LIBXSMM_INTRINSICS_DEBUG)
# include "immintrin_dbg.h"
# define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CPX
# if !defined(_mm512_undefined_epi32)
#   define _mm512_undefined_epi32() _mm512_set1_epi32(0)
# endif
# if !defined(_mm256_movemask_epi8)
# define _mm256_movemask_epi8 mm256_movemask_epi8_dbg
  LIBXSMM_API_INLINE int mm256_movemask_epi8_dbg(__m256i k) {
    unsigned char mask[32], i; int result = 0;
    _mm256_storeu_si256((__m256i*)mask, k);
    for (i = 0; i < 32; ++i) result |= (mask[i] >> 7) << i;
    return result;
  }
# endif
# if !defined(_mm512_and_epi32)
# define _mm512_and_epi32 mm512_and_epi32_dbg
  LIBXSMM_API_INLINE __m512i mm512_and_epi32_dbg(__m512i a, __m512i b) {
    uint32_t a16[16], b16[16]; signed char i;
    _mm512_storeu_si512((__m512i*)a16, a);
    _mm512_storeu_si512((__m512i*)b16, b);
    for (i = 0; i < 16; ++i) a16[i] &= b16[i];
    return _mm512_loadu_si512((const __m512i*)a16);
  }
# endif
# if !defined(_mm512_or_epi32)
# define _mm512_or_epi32 mm512_or_epi32_dbg
  LIBXSMM_API_INLINE __m512i mm512_or_epi32_dbg(__m512i a, __m512i b) {
    uint32_t a16[16], b16[16]; signed char i;
    _mm512_storeu_si512((__m512i*)a16, a);
    _mm512_storeu_si512((__m512i*)b16, b);
    for (i = 0; i < 16; ++i) a16[i] |= b16[i];
    return _mm512_loadu_si512((const __m512i*)a16);
  }
# endif
# if !defined(_mm512_xor_epi32)
# define _mm512_xor_epi32 mm512_xor_epi32_dbg
  LIBXSMM_API_INLINE __m512i mm512_xor_epi32_dbg(__m512i a, __m512i b) {
    uint32_t a16[16], b16[16]; signed char i;
    _mm512_storeu_si512((__m512i*)a16, a);
    _mm512_storeu_si512((__m512i*)b16, b);
    for (i = 0; i < 16; ++i) a16[i] ^= b16[i];
    return _mm512_loadu_si512((const __m512i*)a16);
  }
# endif
# if !defined(_mm512_srli_epi32_dbg) /* GCC: avoid conflict w/ built-in */
# undef _mm512_srli_epi32
# define _mm512_srli_epi32 mm512_srli_epi32_dbg
  LIBXSMM_API_INLINE __m512i mm512_srli_epi32_dbg(__m512i a, unsigned int imm8) {
    uint32_t a16[16]; signed char i;
    _mm512_storeu_si512((__m512i*)a16, a);
    for (i = 0; i < 16; ++i) a16[i] >>= imm8;
    return _mm512_loadu_si512((const __m512i*)a16);
  }
# endif
# if !defined(_mm512_slli_epi32_dbg) /* GCC: avoid conflict w/ built-in */
# undef _mm512_slli_epi32
# define _mm512_slli_epi32 mm512_slli_epi32_dbg
  LIBXSMM_API_INLINE __m512i mm512_slli_epi32_dbg(__m512i a, unsigned int imm8) {
    uint32_t a16[16]; signed char i;
    _mm512_storeu_si512((__m512i*)a16, a);
    for (i = 0; i < 16; ++i) a16[i] <<= imm8;
    return _mm512_loadu_si512((const __m512i*)a16);
  }
# endif
# if !defined(_mm512_sub_ps)
# define _mm512_sub_ps mm512_sub_ps_dbg
  LIBXSMM_API_INLINE __m512 mm512_sub_ps_dbg(__m512 a, __m512 b) {
    float a16[16], b16[16]; signed char i;
    _mm512_storeu_ps((__m512*)a16, a);
    _mm512_storeu_ps((__m512*)b16, b);
    for (i = 0; i < 16; ++i) a16[i] -= b16[i];
    return _mm512_loadu_ps((const __m512*)a16);
  }
# endif
#endif

/** Macro evaluates to LIBXSMM_ATTRIBUTE_TARGET_xxx (see below). */
#define LIBXSMM_ATTRIBUTE_TARGET(TARGET) LIBXSMM_CONCATENATE(LIBXSMM_ATTRIBUTE_TARGET_, TARGET)

#if /*no intrinsics: tested with 17.x and 18.x*/(defined(__PGI) && \
    LIBXSMM_VERSION2(19, 0) > LIBXSMM_VERSION2(__PGIC__, __PGIC_MINOR__)) \
 || /*legacy*/(defined(_CRAYC) && !defined(__GNUC__))
# if !defined(LIBXSMM_INTRINSICS_NONE) && !defined(LIBXSMM_INTRINSICS_STATIC)
#   define LIBXSMM_INTRINSICS_NONE
# endif
#elif !defined(LIBXSMM_INTRINSICS_STATIC) && !defined(LIBXSMM_INTRINSICS_NONE) && ( \
      (defined(__GNUC__) && !defined(__clang__) && !defined(LIBXSMM_INTEL_COMPILER) && !defined(_CRAYC) && \
        LIBXSMM_VERSION2(4, 4) > LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__)) /* GCC 4.4 (target-attribute) */ \
   || (defined(__clang__) && LIBXSMM_VERSION2(3, 7) > LIBXSMM_VERSION2(__clang_major__, __clang_minor__)) \
   || (defined(__APPLE__) && defined(__MACH__) && !defined(LIBXSMM_INTEL_COMPILER) && defined(__clang__) && \
        LIBXSMM_VERSION2(9, 0) > LIBXSMM_VERSION2(__clang_major__, __clang_minor__)))
# define LIBXSMM_INTRINSICS_STATIC
#endif

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(push,target(LIBXSMM_OFFLOAD_TARGET))
#endif

#if defined(__MIC__) && !defined(LIBXSMM_INTRINSICS_NONE)
# if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#   define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_TARGET_ARCH_GENERIC
# endif
# define LIBXSMM_INTRINSICS(TARGET)
# define LIBXSMM_INTRINSICS_INCLUDE
#elif !defined(LIBXSMM_INTRINSICS_NONE) /*!defined(__MIC__)*/
# if    defined(__AVX512F__)  && defined(__AVX512CD__) \
   &&   defined(__AVX512DQ__) && defined(__AVX512BW__) && defined(__AVX512VL__) && defined(__AVX512VNNI__) && defined(__AVX512BF16__) \
   &&   defined(__AVX2__) && defined(__FMA__) && defined(__AVX__) && defined(__SSE4_2__) && defined(__SSE4_1__) && defined(__SSE3__) \
   && (!defined(__GNUC__)  || defined(__clang__) || defined(LIBXSMM_INTEL_COMPILER) || defined(_CRAYC) /* TODO: check GCC, Clang, etc. */ \
                           || (LIBXSMM_VERSION2(10, 0) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__))) \
   && (!defined(__clang__) || (LIBXSMM_VERSION2( 9, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))) \
   && (!defined(__APPLE__) || !defined(__MACH__) || LIBXSMM_VERSION2(99, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
#   if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#     define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CPX
#   endif
#   define LIBXSMM_INTRINSICS_INCLUDE
# elif  defined(__AVX512F__)  && defined(__AVX512CD__) \
   &&   defined(__AVX512DQ__) && defined(__AVX512BW__) && defined(__AVX512VL__) && defined(__AVX512VNNI__) \
   &&   defined(__AVX2__) && defined(__FMA__) && defined(__AVX__) && defined(__SSE4_2__) && defined(__SSE4_1__) && defined(__SSE3__) \
   && (!defined(__GNUC__)  || defined(__clang__) || defined(LIBXSMM_INTEL_COMPILER) || defined(_CRAYC) \
                           || (LIBXSMM_VERSION2(8, 0) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__))) \
   && (!defined(__clang__) || (LIBXSMM_VERSION2(6, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))) \
   && (!defined(__APPLE__) || !defined(__MACH__) || LIBXSMM_VERSION2(10, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
#   if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#     define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CLX
#   endif
#   define LIBXSMM_INTRINSICS_INCLUDE
# elif  defined(__AVX512F__)  && defined(__AVX512CD__) \
   &&   defined(__AVX512DQ__) && defined(__AVX512BW__) && defined(__AVX512VL__) \
   &&   defined(__AVX2__) && defined(__FMA__) && defined(__AVX__) && defined(__SSE4_2__) && defined(__SSE4_1__) && defined(__SSE3__) \
   && (!defined(__GNUC__)  || defined(__clang__) || defined(LIBXSMM_INTEL_COMPILER) || defined(_CRAYC) \
                           || (LIBXSMM_VERSION2(5, 0) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__))) \
   && (!defined(__clang__) || (LIBXSMM_VERSION2(4, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))) \
   && (!defined(__APPLE__) || !defined(__MACH__) || LIBXSMM_VERSION2(9, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
#   if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#     define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CORE
#   endif
#   define LIBXSMM_INTRINSICS_INCLUDE
# elif  defined(__AVX512F__) && defined(__AVX512CD__) \
   &&   defined(__AVX512PF__) && defined(__AVX512ER__) \
   &&   defined(__AVX2__) && defined(__FMA__) && defined(__AVX__) && defined(__SSE4_2__) && defined(__SSE4_1__) && defined(__SSE3__) \
   && (!defined(__GNUC__)  || defined(__clang__) || defined(LIBXSMM_INTEL_COMPILER) || defined(_CRAYC) \
                           || (LIBXSMM_VERSION2(5, 0) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__))) \
   && (!defined(__clang__) || (LIBXSMM_VERSION2(4, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))) \
   && (!defined(__APPLE__) || !defined(__MACH__) || LIBXSMM_VERSION2(9, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
#   if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#     define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_MIC
#   endif
#   define LIBXSMM_INTRINSICS_INCLUDE
# elif  defined(__AVX512F__) && defined(__AVX512CD__) \
   &&   defined(__AVX2__) && defined(__FMA__) && defined(__AVX__) && defined(__SSE4_2__) && defined(__SSE4_1__) && defined(__SSE3__) \
   && (!defined(__GNUC__)  || defined(__clang__) || defined(LIBXSMM_INTEL_COMPILER) || defined(_CRAYC) \
                           || (LIBXSMM_VERSION2(5, 0) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__))) \
   && (!defined(__clang__) || (LIBXSMM_VERSION2(4, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))) \
   && (!defined(__APPLE__) || !defined(__MACH__) || LIBXSMM_VERSION2(9, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
#   if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#     define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512
#   endif
#   define LIBXSMM_INTRINSICS_INCLUDE
# elif defined(__AVX2__) && defined(__FMA__) && defined(__AVX__) && defined(__SSE4_2__) && defined(__SSE4_1__) && defined(__SSE3__)
#   if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#     define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_AVX2
#   endif
#   define LIBXSMM_INTRINSICS_INCLUDE
# elif defined(__AVX__) && defined(__SSE4_2__) && defined(__SSE4_1__) && defined(__SSE3__)
#   if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#     define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_AVX
#   endif
#   define LIBXSMM_INTRINSICS_INCLUDE
# elif defined(__SSE4_2__) && defined(__SSE4_1__) && defined(__SSE3__)
#   if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#     define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_SSE4
#   endif
#   define LIBXSMM_INTRINSICS_INCLUDE
# elif defined(__SSE3__)
#   if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#     define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_SSE3
#   endif
#   define LIBXSMM_INTRINSICS_INCLUDE
# elif defined(LIBXSMM_PLATFORM_X86)
#   if !defined(LIBXSMM_STATIC_TARGET_ARCH)
#     define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_X86_GENERIC
#   endif
#   if defined(__GNUC__)
#     define LIBXSMM_INTRINSICS_INCLUDE
#   endif
# endif
# if defined(LIBXSMM_STATIC_TARGET_ARCH) && !defined(LIBXSMM_INTRINSICS_STATIC)
#   if defined(__INTEL_COMPILER)
      /* TODO: compiler version check for LIBXSMM_MAX_STATIC_TARGET_ARCH */
#     if 1904 <= (LIBXSMM_INTEL_COMPILER)
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CPX
#     elif 1801 <= (LIBXSMM_INTEL_COMPILER)
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CLX
#     elif 1500 <= (LIBXSMM_INTEL_COMPILER)
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CORE
#     elif 1400 <= (LIBXSMM_INTEL_COMPILER)
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_MIC
#     else
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX2
#     endif
#     define LIBXSMM_INTRINSICS(TARGET)/*no need for target flags*/
#     define LIBXSMM_INTRINSICS_INCLUDE
#   elif defined(_CRAYC) && defined(__GNUC__)
      /* TODO: version check e.g., LIBXSMM_VERSION2(11, 5) <= LIBXSMM_VERSION2(_RELEASE, _RELEASE_MINOR) */
#     define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX
#     define LIBXSMM_INTRINSICS(TARGET)/*no need for target flags*/
#     define LIBXSMM_INTRINSICS_INCLUDE
#   elif defined(_MSC_VER) && !defined(__clang__)
      /* TODO: compiler version check for LIBXSMM_MAX_STATIC_TARGET_ARCH */
#     define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX2
#     define LIBXSMM_INTRINSICS(TARGET)/*no need for target flags*/
#     define LIBXSMM_INTRINSICS_INCLUDE
#   elif (!defined(__GNUC__)  || LIBXSMM_VERSION2(4, 9) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__)) \
      && (!defined(__clang__) || LIBXSMM_VERSION2(4, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__)) \
      && (!defined(__APPLE__) || !defined(__MACH__)) && !defined(__PGI) && !defined(_MSC_VER)
#     if defined(__CYGWIN__) && !defined(LIBXSMM_INTRINSICS_DEBUG) /* Cygwin: invalid register for .seh_savexmm */
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX2
#     elif (defined(__GNUC__)  && LIBXSMM_VERSION2(10, 0) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__)) \
        || (defined(__clang__) && LIBXSMM_VERSION2( 9, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CPX
#     elif (defined(__GNUC__)  && LIBXSMM_VERSION2(8, 0) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__)) \
        || (defined(__clang__) && LIBXSMM_VERSION2(6, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CLX
#     elif (defined(__GNUC__)  && LIBXSMM_VERSION2(5, 0) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__)) \
        || (defined(__clang__) && LIBXSMM_VERSION2(6, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CORE
#     else
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX2
#     endif
#     define LIBXSMM_INTRINSICS_INCLUDE
#   else /* GCC/legacy incl. Clang */
#     if defined(__clang__) && !(defined(__APPLE__) && defined(__MACH__)) && !defined(_WIN32)
#       if (LIBXSMM_VERSION2(7, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__)) /* TODO */
          /* no limitations */
#       elif (LIBXSMM_VERSION2(4, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
#         if !defined(LIBXSMM_INTRINSICS_STATIC) && (LIBXSMM_STATIC_TARGET_ARCH < LIBXSMM_X86_AVX2/*workaround*/)
#           define LIBXSMM_INTRINSICS_STATIC
#         endif
#       elif !defined(LIBXSMM_INTRINSICS_STATIC)
#         define LIBXSMM_INTRINSICS_STATIC
#       endif
#       if defined(__CYGWIN__) && !defined(LIBXSMM_INTRINSICS_DEBUG) /* Cygwin: invalid register for .seh_savexmm */
#         define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX2
#       elif LIBXSMM_VERSION2(9, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__)
#         define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CPX
#       elif LIBXSMM_VERSION2(6, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__)
#         define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CLX
#       else
#         define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_X86_AVX512_CORE
#       endif
#     else /* fall-back */
#       define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_STATIC_TARGET_ARCH
#       if !defined(LIBXSMM_INTRINSICS_STATIC) && (LIBXSMM_STATIC_TARGET_ARCH < LIBXSMM_X86_AVX2/*workaround*/)
#         define LIBXSMM_INTRINSICS_STATIC
#       endif
#     endif
#     if !defined(LIBXSMM_INTRINSICS_INCLUDE) && (!defined(__PGI) || LIBXSMM_VERSION2(19, 0) <= LIBXSMM_VERSION2(__PGIC__, __PGIC_MINOR__))
#       define LIBXSMM_INTRINSICS_INCLUDE
#     endif
#   endif /* GCC/legacy incl. Clang */
#   if !defined(LIBXSMM_MAX_STATIC_TARGET_ARCH)
#     error "LIBXSMM_MAX_STATIC_TARGET_ARCH not defined!"
#   endif
#   if defined(LIBXSMM_INTRINSICS_INCLUDE) && !defined(LIBXSMM_INTRINSICS_NONE) && !defined(LIBXSMM_INTRINSICS_DEBUG)
#     include <immintrin.h>
#   endif /*defined(LIBXSMM_INTRINSICS_INCLUDE)*/
#   if !defined(LIBXSMM_INTRINSICS)
#     if (LIBXSMM_MAX_STATIC_TARGET_ARCH > LIBXSMM_STATIC_TARGET_ARCH)
#       define LIBXSMM_INTRINSICS(TARGET) LIBXSMM_ATTRIBUTE(LIBXSMM_ATTRIBUTE_TARGET(TARGET))
        /* LIBXSMM_ATTRIBUTE_TARGET_xxx is required to literally match the CPUID (libxsmm_cpuid.h)! */
#       define LIBXSMM_ATTRIBUTE_TARGET_1002 target("sse2") /* LIBXSMM_X86_GENERIC (64-bit ABI) */
#       if (LIBXSMM_X86_SSE3 <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
#         define LIBXSMM_ATTRIBUTE_TARGET_1003 target("sse3")
#       else
#         define LIBXSMM_ATTRIBUTE_TARGET_1003 LIBXSMM_ATTRIBUTE_TARGET_1002
#       endif
#       if (LIBXSMM_X86_SSE4 <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
#         define LIBXSMM_ATTRIBUTE_TARGET_1004 target("sse4.1,sse4.2")
#       else
#         define LIBXSMM_ATTRIBUTE_TARGET_1004 LIBXSMM_ATTRIBUTE_TARGET_1003
#       endif
#       if (LIBXSMM_X86_AVX <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
#         define LIBXSMM_ATTRIBUTE_TARGET_1005 target("avx")
#       else
#         define LIBXSMM_ATTRIBUTE_TARGET_1005 LIBXSMM_ATTRIBUTE_TARGET_1004
#       endif
#       if (LIBXSMM_X86_AVX2 <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
#         define LIBXSMM_ATTRIBUTE_TARGET_1006 target("avx2,fma")
#       else
#         define LIBXSMM_ATTRIBUTE_TARGET_1006 LIBXSMM_ATTRIBUTE_TARGET_1005
#       endif
#       if (LIBXSMM_X86_AVX512 <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
#         define LIBXSMM_ATTRIBUTE_TARGET_1007 target("avx2,fma,avx512f,avx512cd")
#       else
#         define LIBXSMM_ATTRIBUTE_TARGET_1007 LIBXSMM_ATTRIBUTE_TARGET_1006
#       endif
#       if (LIBXSMM_X86_AVX512_MIC <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
#         define LIBXSMM_ATTRIBUTE_TARGET_1010 target("avx2,fma,avx512f,avx512cd,avx512pf,avx512er")
#       else /* LIBXSMM_X86_AVX512 */
#         define LIBXSMM_ATTRIBUTE_TARGET_1010 LIBXSMM_ATTRIBUTE_TARGET_1007
#       endif
#       if (LIBXSMM_X86_AVX512_KNM <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
#         define LIBXSMM_ATTRIBUTE_TARGET_1011 target("avx2,fma,avx512f,avx512cd,avx512pf,avx512er,avx5124vnniw,avx5124fmaps")
#       else /* LIBXSMM_X86_AVX512_MIC */
#         define LIBXSMM_ATTRIBUTE_TARGET_1011 LIBXSMM_ATTRIBUTE_TARGET_1010
#       endif
#       if (LIBXSMM_X86_AVX512_CORE <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
#         define LIBXSMM_ATTRIBUTE_TARGET_1020 target("avx2,fma,avx512f,avx512cd,avx512dq,avx512bw,avx512vl")
#       else /* LIBXSMM_X86_AVX512 */
#         define LIBXSMM_ATTRIBUTE_TARGET_1020 LIBXSMM_ATTRIBUTE_TARGET_1007
#       endif
#       if (LIBXSMM_X86_AVX512_CLX <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
#         define LIBXSMM_ATTRIBUTE_TARGET_1021 target("avx2,fma,avx512f,avx512cd,avx512dq,avx512bw,avx512vl,avx512vnni")
#       else /* LIBXSMM_X86_AVX512_CORE */
#         define LIBXSMM_ATTRIBUTE_TARGET_1021 LIBXSMM_ATTRIBUTE_TARGET_1020
#       endif
#       if (LIBXSMM_X86_AVX512_CPX <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
#         define LIBXSMM_ATTRIBUTE_TARGET_1022 target("avx2,fma,avx512f,avx512cd,avx512dq,avx512bw,avx512vl,avx512vnni,avx512bf16")
#       else /* LIBXSMM_X86_AVX512_CORE */
#         define LIBXSMM_ATTRIBUTE_TARGET_1022 LIBXSMM_ATTRIBUTE_TARGET_1021
#       endif
#     else
#       define LIBXSMM_INTRINSICS(TARGET)/*no need for target flags*/
#     endif
#   elif !defined(LIBXSMM_INTRINSICS_TARGET)
#     define LIBXSMM_INTRINSICS_TARGET
#   endif /*!defined(LIBXSMM_INTRINSICS)*/
# endif /*defined(LIBXSMM_STATIC_TARGET_ARCH)*/
#endif /*!defined(LIBXSMM_INTRINSICS_NONE)*/

#if !defined(LIBXSMM_STATIC_TARGET_ARCH)
# if !defined(LIBXSMM_INTRINSICS_NONE) && !defined(LIBXSMM_INTRINSICS_STATIC)
#   define LIBXSMM_INTRINSICS_NONE
# endif
# define LIBXSMM_STATIC_TARGET_ARCH LIBXSMM_TARGET_ARCH_GENERIC
#endif

#if !defined(LIBXSMM_MAX_STATIC_TARGET_ARCH)
# define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_STATIC_TARGET_ARCH
#elif (LIBXSMM_MAX_STATIC_TARGET_ARCH < LIBXSMM_STATIC_TARGET_ARCH)
# undef LIBXSMM_MAX_STATIC_TARGET_ARCH
# define LIBXSMM_MAX_STATIC_TARGET_ARCH LIBXSMM_STATIC_TARGET_ARCH
#endif

#if !defined(LIBXSMM_INTRINSICS)
# define LIBXSMM_INTRINSICS(TARGET)
#endif

/** Include basic x86 intrinsics such as __rdtsc. */
#if defined(LIBXSMM_INTRINSICS_INCLUDE) && !defined(LIBXSMM_INTRINSICS_DEBUG)
# if defined(_WIN32)
#   include <intrin.h>
# elif defined(LIBXSMM_INTEL_COMPILER) || defined(_CRAYC) || defined(__clang__) || defined(__PGI)
#   include <x86intrin.h>
# elif defined(__GNUC__) && (LIBXSMM_VERSION2(4, 4) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__))
#   include <x86intrin.h>
# endif
# include <xmmintrin.h>
# if defined(__SSE3__)
#   include <pmmintrin.h>
# endif
#endif

#if !defined(LIBXSMM_INTRINSICS_NONE)
# if defined(_WIN32)
#   include <malloc.h>
# else
#   include <mm_malloc.h>
# endif
#endif

/**
 * Intrinsic-specific fix-ups
 */
#if defined(__clang__)
# define LIBXSMM_INTRINSICS_LDDQU_SI128(A) _mm_loadu_si128(A)
#else
# define LIBXSMM_INTRINSICS_LDDQU_SI128(A) _mm_lddqu_si128(A)
#endif
#if !defined(LIBXSMM_INTEL_COMPILER) && defined(__clang__) && ( \
      (LIBXSMM_VERSION2(3, 9) > LIBXSMM_VERSION2(__clang_major__, __clang_minor__)) \
   || (LIBXSMM_VERSION2(7, 3) > LIBXSMM_VERSION2(__clang_major__, __clang_minor__) && \
       defined(__APPLE__) && defined(__MACH__)))
/* prototypes with incorrect signature: _mm512_load_ps takes DP*, _mm512_load_pd takes SP* (checked with v3.8.1) */
# define LIBXSMM_INTRINSICS_MM512_LOAD_PS(A) _mm512_load_ps((const double*)(A))
# define LIBXSMM_INTRINSICS_MM512_LOAD_PD(A) _mm512_load_pd((const float*)(A))
/* Clang misses _mm512_stream_p? (checked with v3.8.1). */
# define LIBXSMM_INTRINSICS_MM512_STREAM_SI512(A, B) _mm512_store_si512(A, B)
# define LIBXSMM_INTRINSICS_MM512_STREAM_PS(A, B) _mm512_store_ps(A, B)
# define LIBXSMM_INTRINSICS_MM512_STREAM_PD(A, B) _mm512_store_pd(A, B)
#else
# define LIBXSMM_INTRINSICS_MM512_LOAD_PS(A) _mm512_load_ps((const float*)(A))
# define LIBXSMM_INTRINSICS_MM512_LOAD_PD(A) _mm512_load_pd((const double*)(A))
# define LIBXSMM_INTRINSICS_MM512_STREAM_SI512(A, B) _mm512_stream_si512((__m512i*)(A), (B))
# define LIBXSMM_INTRINSICS_MM512_STREAM_PS(A, B) _mm512_stream_ps(A, B)
# define LIBXSMM_INTRINSICS_MM512_STREAM_PD(A, B) _mm512_stream_pd(A, B)
#endif
#if !defined(LIBXSMM_INTEL_COMPILER) || (defined(__clang__) && ( \
      (LIBXSMM_VERSION2(8, 0) > LIBXSMM_VERSION2(__clang_major__, __clang_minor__)))) \
   || (defined(__APPLE__) && defined(__MACH__)) || defined(__GNUC__)
# define LIBXSMM_INTRINSICS_MM256_STORE_EPI32(A, B) _mm256_store_si256((__m256i*)(A), B)
#else
# define LIBXSMM_INTRINSICS_MM256_STORE_EPI32(A, B) _mm256_store_epi32(A, B)
#endif
#if defined(LIBXSMM_INTEL_COMPILER)
# if 1600 <= (LIBXSMM_INTEL_COMPILER)
#   define LIBXSMM_INTRINSICS_MM512_SET_EPI16(E31, E30, E29, E28, E27, E26, E25, E24, E23, E22, E21, E20, E19, E18, E17, E16, \
                                                        E15, E14, E13, E12, E11, E10, E9, E8, E7, E6, E5, E4, E3, E2, E1, E0) \
                             _mm512_set_epi16(E31, E30, E29, E28, E27, E26, E25, E24, E23, E22, E21, E20, E19, E18, E17, E16, \
                                                        E15, E14, E13, E12, E11, E10, E9, E8, E7, E6, E5, E4, E3, E2, E1, E0)
# else
#   define LIBXSMM_INTRINSICS_MM512_SET_EPI16(E31, E30, E29, E28, E27, E26, E25, E24, E23, E22, E21, E20, E19, E18, E17, E16, \
                                                        E15, E14, E13, E12, E11, E10, E9, E8, E7, E6, E5, E4, E3, E2, E1, E0) \
         _mm512_castps_si512(_mm512_set_epi16(E31, E30, E29, E28, E27, E26, E25, E24, E23, E22, E21, E20, E19, E18, E17, E16, \
                                                        E15, E14, E13, E12, E11, E10, E9, E8, E7, E6, E5, E4, E3, E2, E1, E0))
# endif
#else
# define LIBXSMM_INTRINSICS_MM512_SET_EPI16(E31, E30, E29, E28, E27, E26, E25, E24, E23, E22, E21, E20, E19, E18, E17, E16, \
                                                      E15, E14, E13, E12, E11, E10, E9, E8, E7, E6, E5, E4, E3, E2, E1, E0) \
               _mm512_set_epi32(((E31) << 16) | (E30), ((E29) << 16) | (E28), ((E27) << 16) | (E26), ((E25) << 16) | (E24), \
                                ((E23) << 16) | (E22), ((E21) << 16) | (E20), ((E19) << 16) | (E18), ((E17) << 16) | (E16), \
                                ((E15) << 16) | (E14), ((E13) << 16) | (E12), ((E11) << 16) | (E10),  ((E9) << 16) |  (E8), \
                                 ((E7) << 16) |  (E6),  ((E5) << 16) |  (E4),  ((E3) << 16) |  (E2),  ((E1) << 16) |  (E0))
#endif
#if defined(LIBXSMM_INTEL_COMPILER) \
  || (defined(__GNUC__) && LIBXSMM_VERSION2(7, 0) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__)) \
  || (defined(__clang__) && (!defined(__APPLE__) || !defined(__MACH__)) \
      && LIBXSMM_VERSION2(4, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
# define LIBXSMM_INTRINSICS_MM512_MASK_I32GATHER_EPI32(A, B, C, D, E) _mm512_mask_i32gather_epi32(A, B, C, D, E)
# define LIBXSMM_INTRINSICS_MM512_EXTRACTI64X4_EPI64(A, B) _mm512_extracti64x4_epi64(A, B)
# define LIBXSMM_INTRINSICS_MM512_ABS_PS(A) _mm512_abs_ps(A)
# define LIBXSMM_INTRINSICS_MM512_UNDEFINED_EPI32() _mm512_undefined_epi32()
# define LIBXSMM_INTRINSICS_MM512_UNDEFINED() _mm512_undefined()
# define LIBXSMM_INTRINSICS_MM_UNDEFINED_PD() _mm_undefined_pd()
#else
# define LIBXSMM_INTRINSICS_MM512_MASK_I32GATHER_EPI32(A, B, C, D, E) _mm512_castps_si512(_mm512_mask_i32gather_ps( \
                           _mm512_castsi512_ps(A), B, C, (const float*)(D), E))
# define LIBXSMM_INTRINSICS_MM512_EXTRACTI64X4_EPI64(A, B) _mm256_castpd_si256(_mm512_extractf64x4_pd(_mm512_castsi512_pd(A), B))
# define LIBXSMM_INTRINSICS_MM512_ABS_PS(A) _mm512_castsi512_ps(_mm512_and_epi32( \
                           _mm512_castps_si512(A), _mm512_set1_epi32(0x7FFFFFFF)))
# define LIBXSMM_INTRINSICS_MM512_UNDEFINED_EPI32() _mm512_set1_epi32(0)
# define LIBXSMM_INTRINSICS_MM512_UNDEFINED() _mm512_set1_ps(0)
# define LIBXSMM_INTRINSICS_MM_UNDEFINED_PD() _mm_set1_pd(0)
#endif
#if (defined(LIBXSMM_INTEL_COMPILER) && (1800 <= (LIBXSMM_INTEL_COMPILER))) \
  || (!defined(LIBXSMM_INTEL_COMPILER) && defined(__GNUC__) \
      && LIBXSMM_VERSION2(7, 0) <= LIBXSMM_VERSION2(__GNUC__, __GNUC_MINOR__)) \
  || ((!defined(__APPLE__) || !defined(__MACH__)) && defined(__clang__) \
      && LIBXSMM_VERSION2(8, 0) <= LIBXSMM_VERSION2(__clang_major__, __clang_minor__))
# define LIBXSMM_INTRINSICS_MM512_STORE_MASK(DST_PTR, SRC, NBITS) \
    LIBXSMM_CONCATENATE(_store_mask, NBITS)((LIBXSMM_CONCATENATE(__mmask, NBITS)*)(DST_PTR), SRC)
# define LIBXSMM_INTRINSICS_MM512_LOAD_MASK(SRC_PTR, NBITS) \
    LIBXSMM_CONCATENATE(_load_mask, NBITS)((/*const*/ LIBXSMM_CONCATENATE(__mmask, NBITS)*)(SRC_PTR))
# define LIBXSMM_INTRINSICS_MM512_CVTU32_MASK(A, NBITS) LIBXSMM_CONCATENATE(_cvtu32_mask, NBITS)((unsigned int)(A))
#elif defined(LIBXSMM_INTEL_COMPILER)
# define LIBXSMM_INTRINSICS_MM512_STORE_MASK(DST_PTR, SRC, NBITS) \
    (*(LIBXSMM_CONCATENATE(__mmask, NBITS)*)(DST_PTR) = (LIBXSMM_CONCATENATE(__mmask, NBITS))(SRC))
# define LIBXSMM_INTRINSICS_MM512_LOAD_MASK(SRC_PTR, NBITS) \
    ((LIBXSMM_CONCATENATE(__mmask, NBITS))_mm512_mask2int(*(const __mmask16*)(SRC_PTR)))
# define LIBXSMM_INTRINSICS_MM512_CVTU32_MASK(A, NBITS) LIBXSMM_CONCATENATE(LIBXSMM_INTRINSICS_MM512_CVTU32_MASK_, NBITS)(A)
# define LIBXSMM_INTRINSICS_MM512_CVTU32_MASK_32(A) ((__mmask32)(A))
# define LIBXSMM_INTRINSICS_MM512_CVTU32_MASK_16(A) _mm512_int2mask((int)(A))
# define LIBXSMM_INTRINSICS_MM512_CVTU32_MASK_8(A) ((__mmask8)(A))
#else
# define LIBXSMM_INTRINSICS_MM512_STORE_MASK(DST_PTR, SRC, NBITS) \
    (*(LIBXSMM_CONCATENATE(__mmask, NBITS)*)(DST_PTR) = (LIBXSMM_CONCATENATE(__mmask, NBITS))(SRC))
# define LIBXSMM_INTRINSICS_MM512_LOAD_MASK(SRC_PTR, NBITS) (*(const LIBXSMM_CONCATENATE(__mmask, NBITS)*)(SRC_PTR))
# define LIBXSMM_INTRINSICS_MM512_CVTU32_MASK(A, NBITS) ((LIBXSMM_CONCATENATE(__mmask, NBITS))(A))
#endif
#define LIBXSMM_INTRINSICS_MM512_STORE_MASK64(DST_PTR, SRC) LIBXSMM_INTRINSICS_MM512_STORE_MASK(DST_PTR, SRC, 64)
#define LIBXSMM_INTRINSICS_MM512_STORE_MASK32(DST_PTR, SRC) LIBXSMM_INTRINSICS_MM512_STORE_MASK(DST_PTR, SRC, 32)
#define LIBXSMM_INTRINSICS_MM512_STORE_MASK16(DST_PTR, SRC) LIBXSMM_INTRINSICS_MM512_STORE_MASK(DST_PTR, SRC, 16)
#define LIBXSMM_INTRINSICS_MM512_STORE_MASK8(DST_PTR, SRC) LIBXSMM_INTRINSICS_MM512_STORE_MASK(DST_PTR, SRC, 8)
#define LIBXSMM_INTRINSICS_MM512_LOAD_MASK64(SRC_PTR) LIBXSMM_INTRINSICS_MM512_LOAD_MASK(SRC_PTR, 64)
#define LIBXSMM_INTRINSICS_MM512_LOAD_MASK32(SRC_PTR) LIBXSMM_INTRINSICS_MM512_LOAD_MASK(SRC_PTR, 32)
#define LIBXSMM_INTRINSICS_MM512_LOAD_MASK16(SRC_PTR) LIBXSMM_INTRINSICS_MM512_LOAD_MASK(SRC_PTR, 16)
#define LIBXSMM_INTRINSICS_MM512_LOAD_MASK8(SRC_PTR) LIBXSMM_INTRINSICS_MM512_LOAD_MASK(SRC_PTR, 8)
#define LIBXSMM_INTRINSICS_MM512_CVTU32_MASK32(A) LIBXSMM_INTRINSICS_MM512_CVTU32_MASK(A, 32)
#define LIBXSMM_INTRINSICS_MM512_CVTU32_MASK16(A) LIBXSMM_INTRINSICS_MM512_CVTU32_MASK(A, 16)
#define LIBXSMM_INTRINSICS_MM512_CVTU32_MASK8(A) LIBXSMM_INTRINSICS_MM512_CVTU32_MASK(A, 8)

/**
 * Pseudo intrinsics for portability
 */
LIBXSMM_API_INLINE int LIBXSMM_INTRINSICS_BITSCANFWD32_SW(unsigned int n) {
  unsigned int i, r = 0; if (0 != n) for (i = 1; 0 == (n & i); i <<= 1) { ++r; } return r;
}
LIBXSMM_API_INLINE int LIBXSMM_INTRINSICS_BITSCANFWD64_SW(unsigned long long n) {
  unsigned int i, r = 0; if (0 != n) for (i = 1; 0 == (n & i); i <<= 1) { ++r; } return r;
}

/** Binary Logarithm (based on Stackoverflow's NBITSx macro). */
#define LIBXSMM_INTRINSICS_BITSCANBWD_SW02(N) (0 != ((N) & 0x2/*0b10*/) ? 1 : 0)
#define LIBXSMM_INTRINSICS_BITSCANBWD_SW04(N) (0 != ((N) & 0xC/*0b1100*/) ? (2 | LIBXSMM_INTRINSICS_BITSCANBWD_SW02((N) >> 2)) : LIBXSMM_INTRINSICS_BITSCANBWD_SW02(N))
#define LIBXSMM_INTRINSICS_BITSCANBWD_SW08(N) (0 != ((N) & 0xF0/*0b11110000*/) ? (4 | LIBXSMM_INTRINSICS_BITSCANBWD_SW04((N) >> 4)) : LIBXSMM_INTRINSICS_BITSCANBWD_SW04(N))
#define LIBXSMM_INTRINSICS_BITSCANBWD_SW16(N) (0 != ((N) & 0xFF00) ? (8 | LIBXSMM_INTRINSICS_BITSCANBWD_SW08((N) >> 8)) : LIBXSMM_INTRINSICS_BITSCANBWD_SW08(N))
#define LIBXSMM_INTRINSICS_BITSCANBWD_SW32(N) (0 != ((N) & 0xFFFF0000) ? (16 | LIBXSMM_INTRINSICS_BITSCANBWD_SW16((N) >> 16)) : LIBXSMM_INTRINSICS_BITSCANBWD_SW16(N))
#define LIBXSMM_INTRINSICS_BITSCANBWD_SW64(N) (0 != ((N) & 0xFFFFFFFF00000000) ? (32 | LIBXSMM_INTRINSICS_BITSCANBWD_SW32((N) >> 32)) : LIBXSMM_INTRINSICS_BITSCANBWD_SW32(N))
#define LIBXSMM_INTRINSICS_BITSCANBWD32_SW(N) LIBXSMM_INTRINSICS_BITSCANBWD_SW32((unsigned int)(N))
#define LIBXSMM_INTRINSICS_BITSCANBWD64_SW(N) LIBXSMM_INTRINSICS_BITSCANBWD_SW64((unsigned long long)(N))

#if defined(_WIN32) && !defined(LIBXSMM_INTRINSICS_NONE)
  LIBXSMM_API_INLINE unsigned int LIBXSMM_INTRINSICS_BITSCANFWD32(unsigned int n) {
    unsigned long r = 0; _BitScanForward(&r, n); return (0 != n) * r;
  }
  LIBXSMM_API_INLINE unsigned int LIBXSMM_INTRINSICS_BITSCANBWD32(unsigned int n) {
    unsigned long r = 0; _BitScanReverse(&r, n); return r;
  }
# if defined(_WIN64)
  LIBXSMM_API_INLINE unsigned int LIBXSMM_INTRINSICS_BITSCANFWD64(unsigned long long n) {
    unsigned long r = 0; _BitScanForward64(&r, n); return (0 != n) * r;
  }
  LIBXSMM_API_INLINE unsigned int LIBXSMM_INTRINSICS_BITSCANBWD64(unsigned long long n) {
    unsigned long r = 0; _BitScanReverse64(&r, n); return r;
  }
# else
# define LIBXSMM_INTRINSICS_BITSCANFWD64 LIBXSMM_INTRINSICS_BITSCANFWD64_SW
# define LIBXSMM_INTRINSICS_BITSCANBWD64 LIBXSMM_INTRINSICS_BITSCANBWD64_SW
# endif
#elif defined(__GNUC__) && !defined(LIBXSMM_INTRINSICS_NONE)
# define LIBXSMM_INTRINSICS_BITSCANFWD32(N) ((0 != (N)) * __builtin_ctz(N))
# define LIBXSMM_INTRINSICS_BITSCANFWD64(N) ((0 != (N)) * __builtin_ctzll(N))
# define LIBXSMM_INTRINSICS_BITSCANBWD32(N) ((0 != (N)) * (31 - __builtin_clz(N)))
# define LIBXSMM_INTRINSICS_BITSCANBWD64(N) ((0 != (N)) * (63 - __builtin_clzll(N)))
#else /* fall-back implementation */
# define LIBXSMM_INTRINSICS_BITSCANFWD32 LIBXSMM_INTRINSICS_BITSCANFWD32_SW
# define LIBXSMM_INTRINSICS_BITSCANFWD64 LIBXSMM_INTRINSICS_BITSCANFWD64_SW
# define LIBXSMM_INTRINSICS_BITSCANBWD32 LIBXSMM_INTRINSICS_BITSCANBWD32_SW
# define LIBXSMM_INTRINSICS_BITSCANBWD64 LIBXSMM_INTRINSICS_BITSCANBWD64_SW
#endif

/** LIBXSMM_NBITS determines the minimum number of bits needed to represent N. */
#define LIBXSMM_NBITS(N) (LIBXSMM_INTRINSICS_BITSCANBWD64(N) + LIBXSMM_MIN(1, N))
#define LIBXSMM_ISQRT2(N) ((unsigned int)((1ULL << (LIBXSMM_NBITS(N) >> 1)) /*+ LIBXSMM_MIN(1, N)*/))
/** LIBXSMM_ILOG2 definition matches ceil(log2(N)). */
LIBXSMM_API_INLINE unsigned int LIBXSMM_ILOG2(unsigned long long n) {
  unsigned int result = 0; if (1 < n) {
    const unsigned int m = LIBXSMM_INTRINSICS_BITSCANBWD64(n);
    result = m + ((unsigned int)LIBXSMM_INTRINSICS_BITSCANBWD64(n - 1) == m);
  } return result;
}

/**
 * Target attribution
 */
#if !defined(LIBXSMM_INTRINSICS_KNC) && !defined(LIBXSMM_INTRINSICS_NONE) && defined(__MIC__)
# define LIBXSMM_INTRINSICS_KNC
#endif
/** LIBXSMM_INTRINSICS_X86 is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_X86) && !defined(LIBXSMM_INTRINSICS_NONE) && (LIBXSMM_X86_GENERIC <= LIBXSMM_STATIC_TARGET_ARCH || \
   (!defined(LIBXSMM_INTRINSICS_STATIC) && LIBXSMM_X86_GENERIC <= LIBXSMM_MAX_STATIC_TARGET_ARCH))
# define LIBXSMM_INTRINSICS_X86
#endif
/** LIBXSMM_INTRINSICS_SSE3 is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_SSE3) && !defined(LIBXSMM_INTRINSICS_NONE) && (LIBXSMM_X86_SSE3 <= LIBXSMM_STATIC_TARGET_ARCH || \
   (!defined(LIBXSMM_INTRINSICS_STATIC) && LIBXSMM_X86_SSE3 <= LIBXSMM_MAX_STATIC_TARGET_ARCH))
# define LIBXSMM_INTRINSICS_SSE3
#endif
/** LIBXSMM_INTRINSICS_SSE4 is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_SSE4) && !defined(LIBXSMM_INTRINSICS_NONE) && (LIBXSMM_X86_SSE4 <= LIBXSMM_STATIC_TARGET_ARCH || \
   (!defined(LIBXSMM_INTRINSICS_STATIC) && LIBXSMM_X86_SSE4 <= LIBXSMM_MAX_STATIC_TARGET_ARCH))
# define LIBXSMM_INTRINSICS_SSE4
#endif
/** LIBXSMM_INTRINSICS_AVX is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_AVX) && !defined(LIBXSMM_INTRINSICS_NONE) && (LIBXSMM_X86_AVX <= LIBXSMM_STATIC_TARGET_ARCH || \
   (!defined(LIBXSMM_INTRINSICS_STATIC) && LIBXSMM_X86_AVX <= LIBXSMM_MAX_STATIC_TARGET_ARCH))
# define LIBXSMM_INTRINSICS_AVX
#endif
/** LIBXSMM_INTRINSICS_AVX2 is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_AVX2) && !defined(LIBXSMM_INTRINSICS_NONE) && (LIBXSMM_X86_AVX2 <= LIBXSMM_STATIC_TARGET_ARCH || \
   (!defined(LIBXSMM_INTRINSICS_STATIC) && LIBXSMM_X86_AVX2 <= LIBXSMM_MAX_STATIC_TARGET_ARCH))
# define LIBXSMM_INTRINSICS_AVX2
#endif
/** LIBXSMM_INTRINSICS_AVX512 is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_AVX512) && !defined(LIBXSMM_INTRINSICS_NONE) && (LIBXSMM_X86_AVX512 <= LIBXSMM_STATIC_TARGET_ARCH || \
   (!defined(LIBXSMM_INTRINSICS_STATIC) && LIBXSMM_X86_AVX512 <= LIBXSMM_MAX_STATIC_TARGET_ARCH))
# define LIBXSMM_INTRINSICS_AVX512
#endif
/** LIBXSMM_INTRINSICS_AVX512_MIC is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_AVX512_MIC) && !defined(LIBXSMM_INTRINSICS_NONE) && (LIBXSMM_X86_AVX512_MIC <= LIBXSMM_STATIC_TARGET_ARCH || \
   (!defined(LIBXSMM_INTRINSICS_STATIC) && LIBXSMM_X86_AVX512_MIC <= LIBXSMM_MAX_STATIC_TARGET_ARCH))
# define LIBXSMM_INTRINSICS_AVX512_MIC
#endif
/** LIBXSMM_INTRINSICS_AVX512_KNM is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_AVX512_KNM) && !defined(LIBXSMM_INTRINSICS_NONE) && (LIBXSMM_X86_AVX512_KNM <= LIBXSMM_STATIC_TARGET_ARCH || \
   (!defined(LIBXSMM_INTRINSICS_STATIC) && LIBXSMM_X86_AVX512_KNM <= LIBXSMM_MAX_STATIC_TARGET_ARCH))
# define LIBXSMM_INTRINSICS_AVX512_KNM
#endif
/** LIBXSMM_INTRINSICS_AVX512_CORE is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_AVX512_CORE) && !defined(LIBXSMM_INTRINSICS_NONE) && (LIBXSMM_X86_AVX512_CORE <= LIBXSMM_STATIC_TARGET_ARCH || \
   (!defined(LIBXSMM_INTRINSICS_STATIC) && LIBXSMM_X86_AVX512_CORE <= LIBXSMM_MAX_STATIC_TARGET_ARCH))
# define LIBXSMM_INTRINSICS_AVX512_CORE
#endif
/** LIBXSMM_INTRINSICS_AVX512_CLX is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_AVX512_CLX) && !defined(LIBXSMM_INTRINSICS_NONE) && (LIBXSMM_X86_AVX512_CLX <= LIBXSMM_STATIC_TARGET_ARCH || \
   (!defined(LIBXSMM_INTRINSICS_STATIC) && LIBXSMM_X86_AVX512_CLX <= LIBXSMM_MAX_STATIC_TARGET_ARCH))
# define LIBXSMM_INTRINSICS_AVX512_CLX
#endif
/** LIBXSMM_INTRINSICS_AVX512_CPX is defined only if the compiler is able to generate this code without special flags. */
#if !defined(LIBXSMM_INTRINSICS_AVX512_CPX) && !defined(LIBXSMM_INTRINSICS_NONE) && defined(LIBXSMM_X86_AVX512_CPX) && \
    !defined(LIBXSMM_INTRINSICS_STATIC) && (LIBXSMM_X86_AVX512_CPX <= LIBXSMM_MAX_STATIC_TARGET_ARCH)
# define LIBXSMM_INTRINSICS_AVX512_CPX
#endif

/**
 * Pseudo intrinsics (AVX-512)
 */
#if defined(LIBXSMM_INTRINSICS_AVX512) /*__AVX512F__*/
# define LIBXSMM_INTRINSICS_MM512_QUANTIZE_NEAR_PS_EPI16( A, B ) _mm512_cvtepi32_epi16(_mm512_cvt_roundps_epi32( \
    _mm512_mul_ps(LIBXSMM_INTRINSICS_MM512_LOAD_PS(A), B), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC))

LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512i LIBXSMM_INTRINSICS_MM512_ROUNDNE_BF16(__m512 a) {
  const __m512i vnaninf = _mm512_set1_epi32(0x7f800000), vrneadd = _mm512_set1_epi32(0x00007fff);
  const __m512i vfixup = _mm512_set1_epi32(0x00000001), vfixupmask = _mm512_set1_epi32(0x00010000);
  const __m512i mm512_roundbf16rne_a_ = _mm512_castps_si512(a);
  const __mmask16 mm512_roundbf16rne_mask1_ = _mm512_cmp_epi32_mask(_mm512_and_epi32(mm512_roundbf16rne_a_, vnaninf), vnaninf, _MM_CMPINT_NE);
  const __mmask16 mm512_roundbf16rne_mask2_ = _mm512_cmp_epi32_mask(_mm512_and_epi32(mm512_roundbf16rne_a_, vfixupmask), vfixupmask, _MM_CMPINT_EQ);
  return _mm512_mask_add_epi32(mm512_roundbf16rne_a_, mm512_roundbf16rne_mask1_, mm512_roundbf16rne_a_, _mm512_mask_add_epi32(vrneadd, mm512_roundbf16rne_mask2_, vrneadd, vfixup));
}

LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512i LIBXSMM_INTRINSICS_MM512_CVT2_FP32_BF16(__m512 a, __m512 b) {
  const __m256i aa = _mm512_cvtepi32_epi16(_mm512_srai_epi32(LIBXSMM_INTRINSICS_MM512_ROUNDNE_BF16(b), 16));
  const __m256i bb = _mm512_cvtepi32_epi16(_mm512_srai_epi32(LIBXSMM_INTRINSICS_MM512_ROUNDNE_BF16(a), 16));
  return _mm512_inserti64x4(_mm512_inserti64x4(_mm512_setzero_si512(), aa, 0), bb, 1);
}

/** SVML-intrinsics */
LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512 LIBXSMM_INTRINSICS_MM512_TANH_PS_RATIONAL_78( __m512 x ) {
  const  __m512 c0        = _mm512_set1_ps(2027025.0f);
  const  __m512 c1        = _mm512_set1_ps(270270.0f);
  const  __m512 c2        = _mm512_set1_ps(6930.0f);
  const  __m512 c3        = _mm512_set1_ps(36.0f);
  const  __m512 c1_d      = _mm512_set1_ps(945945.0f);
  const  __m512 c2_d      = _mm512_set1_ps(51975.0f);
  const  __m512 c3_d      = _mm512_set1_ps(630.0f);
  const  __m512 hi_bound  = _mm512_set1_ps(4.97f);
  const  __m512 lo_bound  = _mm512_set1_ps(-4.97f);
  const  __m512 ones      = _mm512_set1_ps(1.0f);
  const  __m512 neg_ones  = _mm512_set1_ps(-1.0f);

  const __m512 x2         = _mm512_mul_ps( x, x );
  const __m512 t1_nom     = _mm512_fmadd_ps( c3, x2, c2 );
  const __m512 t2_nom     = _mm512_fmadd_ps( t1_nom, x2, c1 );
  const __m512 t3_nom     = _mm512_fmadd_ps( t2_nom, x2, c0 );
  const __m512 nom        = _mm512_mul_ps( t3_nom, x );
  const __m512 t1_denom   = _mm512_add_ps( x2, c3_d );
  const __m512 t2_denom   = _mm512_fmadd_ps( t1_denom, x2, c2_d );
  const __m512 t3_denom   = _mm512_fmadd_ps( t2_denom, x2, c1_d );
  const __m512 denom      = _mm512_fmadd_ps( t3_denom, x2, c0 );
  const __m512 denom_rcp  = _mm512_rcp14_ps( denom );
  const __mmask16 mask_hi = _mm512_cmp_ps_mask( x, hi_bound, _CMP_GT_OQ);
  const __mmask16 mask_lo = _mm512_cmp_ps_mask( x, lo_bound, _CMP_LT_OQ);
  __m512 result           = _mm512_mul_ps( nom, denom_rcp );
  result                  = _mm512_mask_blend_ps(mask_hi, result, ones);
  result                  = _mm512_mask_blend_ps(mask_lo, result, neg_ones);

  return result;
}

LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512 LIBXSMM_INTRINSICS_MM512_TANH_PS_RATIONAL_32( __m512 x ) {
  const  __m512 c1        = _mm512_set1_ps((float)(1.0/27.0));
  const  __m512 c2        = _mm512_set1_ps((float)(1.0/3));
  const  __m512 hi_bound  = _mm512_set1_ps(3.2f);
  const  __m512 lo_bound  = _mm512_set1_ps(-3.2f);
  const  __m512 ones      = _mm512_set1_ps(1.0f);
  const  __m512 neg_ones  = _mm512_set1_ps(-1.0f);

  const __m512 x2         = _mm512_mul_ps( x, x );
  const __m512 t1_nom     = _mm512_fmadd_ps( x2, c1, ones);
  const __m512 nom        = _mm512_mul_ps( t1_nom, x );
  const __m512 denom      = _mm512_fmadd_ps( x2, c2, ones);
  const __m512 denom_rcp  = _mm512_rcp14_ps( denom );
  const __mmask16 mask_hi = _mm512_cmp_ps_mask( x, hi_bound, _CMP_GT_OQ);
  const __mmask16 mask_lo = _mm512_cmp_ps_mask( x, lo_bound, _CMP_LT_OQ);
  __m512 result           = _mm512_mul_ps(nom, denom_rcp);
  result                  = _mm512_mask_blend_ps(mask_hi, result, ones);
  result                  = _mm512_mask_blend_ps(mask_lo, result, neg_ones);

  return result;
}

LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512 LIBXSMM_INTRINSICS_MM512_TANH_PS_EXP2( __m512 _x ) {
  const __m512 twice_log2_e = _mm512_set1_ps((float)(1.442695*2));
  const __m512 half       = _mm512_set1_ps(0.5f);
  const __m512 c2         = _mm512_set1_ps(0.240226507f);
  const __m512 c1         = _mm512_set1_ps(0.452920674f);
  const __m512 c0         = _mm512_set1_ps(0.713483036f);
  const __m512 ones       = _mm512_set1_ps(1.0f);
  const __m512 minus_twos = _mm512_set1_ps(-2.0f);

  const __m512 x          = _mm512_fmadd_ps(_x, twice_log2_e, half);
#if 1
  const __m512 y          = _mm512_sub_ps(x, _mm512_roundscale_round_ps(x, 1, _MM_FROUND_CUR_DIRECTION));
#else
  const __m512 y          = _mm512_reduce_ps(x, 1);
#endif
  const __m512 t1         = _mm512_fmadd_ps( y, c2, c1);
  const __m512 two_to_y   = _mm512_fmadd_ps( y, t1, c0);
  const __m512 exp        = _mm512_scalef_ps( two_to_y, x );
  const __m512 denom_rcp  = _mm512_rcp14_ps( _mm512_add_ps( exp, ones) );
  __m512 result     = _mm512_fmadd_ps( denom_rcp, minus_twos, ones);

 return result;
}

LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512 LIBXSMM_INTRINSICS_MM512_TANH_PS_EXP3( __m512 _x ) {
  const __m512 twice_log2_e = _mm512_set1_ps((float)(1.442695*2));
  const __m512 half       = _mm512_set1_ps(0.5f);
  const __m512 c3         = _mm512_set1_ps(0.05550410866f);
  const __m512 c2         = _mm512_set1_ps(0.15697034396f);
  const __m512 c1         = _mm512_set1_ps(0.49454875509f);
  const __m512 c0         = _mm512_set1_ps(0.70654502287f);
  const __m512 ones       = _mm512_set1_ps(1.0f);
  const __m512 minus_twos = _mm512_set1_ps(-2.0f);

  const __m512 x          = _mm512_fmadd_ps(_x, twice_log2_e, half);
#if 1
  const __m512 y          = _mm512_sub_ps(x, _mm512_roundscale_round_ps(x, 1, _MM_FROUND_CUR_DIRECTION));
#else
  const __m512 y          = _mm512_reduce_ps(x, 1);
#endif
  const __m512 t1         = _mm512_fmadd_ps( y, c3, c2);
  const __m512 t2         = _mm512_fmadd_ps( y, t1, c1);
  const __m512 two_to_y   = _mm512_fmadd_ps( y, t2, c0);
  const __m512 exp        = _mm512_scalef_ps( two_to_y, x );
  const __m512 denom_rcp  = _mm512_rcp14_ps( _mm512_add_ps( exp, ones) );
  __m512 result     = _mm512_fmadd_ps( denom_rcp, minus_twos, ones);

  return result;
}

LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512 LIBXSMM_INTRINSICS_MM512_TANH_PS_MINIMAX2( __m512 x ) {
  __m512 result, func_p0, func_p1, func_p2;
  const __m512i sign_mask = _mm512_set1_epi32( 0x80000000 );
  const __m512i sign_filter = _mm512_set1_epi32( 0x7FFFFFFF );
  const __m512i lut_low = _mm512_set1_epi32( 246 );
  const __m512i lut_high = _mm512_set1_epi32( 261 );
  const __m512 tanh_p0_2_reg = _mm512_set_ps( 0.40555000f,  0.11892800f, -0.00972979f, -0.02740300f, -0.0169851f, -0.00776152f, -0.00305889f,
                                             -0.00116259f, -0.00041726f, -8.53233e-6f,  1.0000000f,  0.99999800f,  0.99975400f,  0.99268200f,
                                              0.93645300f,  0.73833900f);
  const __m512 tanh_p1_2_reg = _mm512_set_ps( 0.495602f, 0.88152f, 1.125700000f, 1.17021000f, 1.1289000000f, 1.07929000f, 1.0432300f, 1.023010f,
                                              1.011620f, 1.00164f, 1.56828e-14f, 4.49924e-7f, 0.0000646924f, 0.00260405f, 0.0311608f, 0.168736f);
  const __m512 tanh_p2_2_reg = _mm512_set_ps(-0.108238f, -0.2384280f, -0.354418000f, -0.38240300f, -0.34135700f, -0.274509000f, -0.20524900f, -0.1511960f,
                                             -0.107635f, -0.0466868f, -3.60822e-16f, -2.05971e-8f, -4.24538e-6f, -0.000231709f, -0.00386434f, -0.0277702f);

  const __m512i signs   = _mm512_and_epi32(_mm512_castps_si512(x), sign_mask);
  const __m512i abs_arg = _mm512_and_epi32(_mm512_castps_si512(x), sign_filter);
  __m512i indices       = _mm512_srli_epi32(abs_arg, 22);
  indices               = _mm512_max_epi32(indices, lut_low);
  indices               = _mm512_min_epi32(indices, lut_high);

  func_p0               = _mm512_permutexvar_ps(indices, tanh_p0_2_reg);
  func_p1               = _mm512_permutexvar_ps(indices, tanh_p1_2_reg);
  func_p2               = _mm512_permutexvar_ps(indices, tanh_p2_2_reg);

  result                = _mm512_fmadd_ps(_mm512_castsi512_ps(abs_arg), func_p2, func_p1);
  result                = _mm512_fmadd_ps(_mm512_castsi512_ps(abs_arg), result, func_p0);
  result                = _mm512_castsi512_ps(_mm512_xor_epi32(_mm512_castps_si512(result), signs));

  return result;
}

LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512 LIBXSMM_INTRINSICS_MM512_TANH_PS_MINIMAX3( __m512 x ) {
  __m512 result, func_p0, func_p1, func_p2, func_p3;
  const __m512i sign_mask = _mm512_set1_epi32( 0x80000000 );
  const __m512i sign_filter = _mm512_set1_epi32( 0x7FFFFFFF );
  const __m512i lut_low = _mm512_set1_epi32( 246 );
  const __m512i lut_high = _mm512_set1_epi32( 261 );

  const __m512 tanh_p0_3_reg = _mm512_setr_ps( 0.466283000f,  0.82850600f,  0.97437500f,  0.99882600f,  0.9999860f,  1.0000000f, -1.50006e-08f, -7.98169e-06f,
                                              -4.53753e-05f, -0.00023755f, -0.00125285f, -0.00572314f, -0.0227717f, -0.0629089f, -0.084234300f,  0.071199800f);
  const __m512 tanh_p1_3_reg = _mm512_setr_ps( 0.500617f, 0.124369f, 0.0137214f, 0.000464124f, 4.02465e-06f, 0.00000f, 1.00001f, 1.00028f, 1.00112f, 1.00414f,
                                               1.015570f, 1.050950f, 1.1478500f, 1.310130000f, 1.378950000f, 1.07407f);
  const __m512 tanh_p2_3_reg = _mm512_setr_ps(-0.16133200f, -0.0305526f, -0.00245909f, -6.12647e-05f, -3.76127e-07f,  0.000000f, -0.000245872f, -0.00341151f,
                                              -0.00971505f, -0.0256817f, -0.06869110f, -0.162433000f, -0.346828000f, -0.566516f, -0.640214000f, -0.44011900f);
  const __m512 tanh_p3_3_reg = _mm512_setr_ps( 0.0177393f,  0.00253432f,  0.000147303f,  2.69963e-06f, 1.16764e-08f, 0.0000000f, -0.330125f, -0.3176210f,
                                              -0.3017760f, -0.27358000f, -0.219375000f, -0.136197000f, -0.01868680f, 0.0808901f,  0.107095f,  0.0631459f);

  const __m512i signs   = _mm512_and_epi32(_mm512_castps_si512(x), sign_mask);
  const __m512i abs_arg = _mm512_and_epi32(_mm512_castps_si512(x), sign_filter);
  __m512i indices       = _mm512_srli_epi32(abs_arg, 22);
  indices               = _mm512_max_epi32(indices, lut_low);
  indices               = _mm512_min_epi32(indices, lut_high);

  func_p0               = _mm512_permutexvar_ps(indices, tanh_p0_3_reg);
  func_p1               = _mm512_permutexvar_ps(indices, tanh_p1_3_reg);
  func_p2               = _mm512_permutexvar_ps(indices, tanh_p2_3_reg);
  func_p3               = _mm512_permutexvar_ps(indices, tanh_p3_3_reg);

  result                = _mm512_fmadd_ps(_mm512_castsi512_ps(abs_arg), func_p3, func_p2);
  result                = _mm512_fmadd_ps(_mm512_castsi512_ps(abs_arg), result, func_p1);
  result                = _mm512_fmadd_ps(_mm512_castsi512_ps(abs_arg), result, func_p0);
  result                = _mm512_castsi512_ps(_mm512_xor_epi32(_mm512_castps_si512(result), signs));

  return result;
}

#if defined(LIBXSMM_INTEL_COMPILER)
# define LIBXSMM_INTRINSICS_MM512_TANH_PS(A) _mm512_tanh_ps(A)
# define LIBXSMM_INTRINSICS_MM512_EXP_PS(A) _mm512_exp_ps(A)
#else
# if !defined(LIBXSMM_NO_LIBM)
#   include <math.h>
# endif
LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512 LIBXSMM_INTRINSICS_MM512_TANH_PS(__m512 a) {
  float a16[16]; int i;
  _mm512_store_ps(a16, a);
  for (i = 0; i < 16; ++i) a16[i] = LIBXSMM_TANHF(a16[i]);
  return _mm512_loadu_ps(a16);
}
LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512 LIBXSMM_INTRINSICS_MM512_EXP_PS(__m512 a) {
  float a16[16]; int i;
  _mm512_store_ps(a16, a);
  for (i = 0; i < 16; ++i) a16[i] = LIBXSMM_EXPF(a16[i]);
  return _mm512_loadu_ps(a16);
}
#endif /* SVML */

/** 2048-bit state for xoshiro128+ RNG */
#define LIBXSMM_INTRINSICS_MM512_RNG_STATE(INDEX) (*(__m512i*)LIBXSMM_CONCATENATE(libxsmm_intrinsics_mm512_rng_state, INDEX))
LIBXSMM_APIVAR_PUBLIC(unsigned int libxsmm_intrinsics_mm512_rng_state0[16]);
LIBXSMM_APIVAR_PUBLIC(unsigned int libxsmm_intrinsics_mm512_rng_state1[16]);
LIBXSMM_APIVAR_PUBLIC(unsigned int libxsmm_intrinsics_mm512_rng_state2[16]);
LIBXSMM_APIVAR_PUBLIC(unsigned int libxsmm_intrinsics_mm512_rng_state3[16]);

/** Generate random number in the interval [0, 1); not thread-safe.
 *  this is based on xoshiro128+ 1.0, e.g. http://prng.di.unimi.it/xoshiro128plus.c */
# if defined(__GNUC__) && !defined(__clang__) && !defined(LIBXSMM_INTEL_COMPILER) && !defined(_CRAYC) && 0
LIBXSMM_PRAGMA_OPTIMIZE_OFF /* avoid ICE in case of symbols (-g) */
# endif
LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512i LIBXSMM_INTRINSICS_MM512_RNG_XOSHIRO128P_EPI32(void) {
  const __m512i result = _mm512_add_epi32(LIBXSMM_INTRINSICS_MM512_RNG_STATE(0), LIBXSMM_INTRINSICS_MM512_RNG_STATE(3));
  const __m512i s = _mm512_slli_epi32(LIBXSMM_INTRINSICS_MM512_RNG_STATE(1), 9);
  __m512i t;
  LIBXSMM_INTRINSICS_MM512_RNG_STATE(2) = _mm512_xor_epi32(LIBXSMM_INTRINSICS_MM512_RNG_STATE(2), LIBXSMM_INTRINSICS_MM512_RNG_STATE(0));
  LIBXSMM_INTRINSICS_MM512_RNG_STATE(3) = _mm512_xor_epi32(LIBXSMM_INTRINSICS_MM512_RNG_STATE(3), LIBXSMM_INTRINSICS_MM512_RNG_STATE(1));
  LIBXSMM_INTRINSICS_MM512_RNG_STATE(1) = _mm512_xor_epi32(LIBXSMM_INTRINSICS_MM512_RNG_STATE(1), LIBXSMM_INTRINSICS_MM512_RNG_STATE(2));
  LIBXSMM_INTRINSICS_MM512_RNG_STATE(0) = _mm512_xor_epi32(LIBXSMM_INTRINSICS_MM512_RNG_STATE(0), LIBXSMM_INTRINSICS_MM512_RNG_STATE(3));
  LIBXSMM_INTRINSICS_MM512_RNG_STATE(2) = _mm512_xor_epi32(LIBXSMM_INTRINSICS_MM512_RNG_STATE(2), s);
  t = _mm512_slli_epi32(LIBXSMM_INTRINSICS_MM512_RNG_STATE(3), 11);
  LIBXSMM_INTRINSICS_MM512_RNG_STATE(3) = _mm512_or_epi32(t, _mm512_srli_epi32(LIBXSMM_INTRINSICS_MM512_RNG_STATE(3), 32 - 11));
  return result;
}

LIBXSMM_API_INLINE LIBXSMM_INTRINSICS(LIBXSMM_X86_AVX512) __m512 LIBXSMM_INTRINSICS_MM512_RNG_PS(void) {
  const __m512i rng_mantissa = _mm512_srli_epi32( LIBXSMM_INTRINSICS_MM512_RNG_XOSHIRO128P_EPI32(), 9 );
  const __m512 one = _mm512_set1_ps(1.0f);
  return _mm512_sub_ps(_mm512_castsi512_ps(_mm512_or_epi32(_mm512_set1_epi32(0x3f800000), rng_mantissa)), one);
}
# if defined(__GNUC__) && !defined(__clang__) && !defined(LIBXSMM_INTEL_COMPILER) && !defined(_CRAYC) && 0
LIBXSMM_PRAGMA_OPTIMIZE_ON
# endif
#endif /*__AVX512F__*/

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(pop)
#endif

#endif /*LIBXSMM_INTRINSICS_X86_H*/

