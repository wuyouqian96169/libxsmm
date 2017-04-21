/******************************************************************************
** Copyright (c) 2016-2017, Intel Corporation                                **
** All rights reserved.                                                      **
**                                                                           **
** Redistribution and use in source and binary forms, with or without        **
** modification, are permitted provided that the following conditions        **
** are met:                                                                  **
** 1. Redistributions of source code must retain the above copyright         **
**    notice, this list of conditions and the following disclaimer.          **
** 2. Redistributions in binary form must reproduce the above copyright      **
**    notice, this list of conditions and the following disclaimer in the    **
**    documentation and/or other materials provided with the distribution.   **
** 3. Neither the name of the copyright holder nor the names of its          **
**    contributors may be used to endorse or promote products derived        **
**    from this software without specific prior written permission.          **
**                                                                           **
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       **
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         **
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR     **
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT      **
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    **
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  **
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    **
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    **
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      **
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        **
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              **
******************************************************************************/
/* Hans Pabst (Intel Corp.)
******************************************************************************/
#ifndef LIBXSMM_TRANS_H
#define LIBXSMM_TRANS_H

#include <libxsmm.h>

/* kernel uses consecutive stores and consecutive loads (copy) */
#define LIBXSMM_MCOPY_KERNEL(TYPE, TYPESIZE, OUT, IN, LDI, LDO, INDEX_I, INDEX_J, SRC, DST) \
  const TYPE *const SRC = (const TYPE*)(((const char*)(IN)) + (TYPESIZE) * ((INDEX_I) * (LDI) + (INDEX_J))); \
  TYPE *const DST = (TYPE*)(((const char*)(OUT)) + (TYPESIZE) * ((INDEX_I) * (LDO) + (INDEX_J)))
/* call JIT-kernel (matrix-copy) */
#define LIBXSMM_MCOPY_CALL_NOPF(KERNEL, TYPESIZE, SRC, LDI, DST, LDO) (KERNEL)(SRC, LDI, DST, LDO)
/* call JIT-kernel (matrix-copy with prefetch) */
#define LIBXSMM_MCOPY_CALL(KERNEL, TYPESIZE, SRC, LDI, DST, LDO) (KERNEL)(SRC, LDI, DST, LDO, \
  ((const char*)(SRC)) + (TYPESIZE) * (*(LDI))) /* prefetch next line*/

/* kernel uses consecutive stores and strided loads (transpose) */
#define LIBXSMM_TCOPY_KERNEL(TYPE, TYPESIZE, OUT, IN, LDI, LDO, INDEX_I, INDEX_J, SRC, DST) \
  const TYPE *const SRC = (const TYPE*)(((const char*)(IN)) + (TYPESIZE) * ((INDEX_J) * (LDI) + (INDEX_I))); \
  TYPE *const DST = (TYPE*)(((const char*)(OUT)) + (TYPESIZE) * ((INDEX_I) * (LDO) + (INDEX_J)))
/* call JIT-kernel (transpose) */
#define LIBXSMM_TCOPY_CALL(KERNEL, TYPESIZE, SRC, LDI, DST, LDO) (KERNEL)(SRC, LDI, DST, LDO)

#define LIBXSMM_XCOPY_LOOP_UNALIGNED(...)
#define LIBXSMM_XCOPY_LOOP(TYPE, TYPESIZE, XKERNEL, HINT_ALIGNED, OUT, IN, LDI, LDO, M0, M1, N0, N1) { \
  /*const*/int generic_type = sizeof(TYPE) == (TYPESIZE); /* used to mute warning (conditional is constant) */ \
  unsigned int libxsmm_xcopy_loop_i_, libxsmm_xcopy_loop_j_; \
  if (generic_type) { \
    for (libxsmm_xcopy_loop_i_ = M0; libxsmm_xcopy_loop_i_ < (M1); ++libxsmm_xcopy_loop_i_) { \
      LIBXSMM_PRAGMA_NONTEMPORAL HINT_ALIGNED(OUT) \
      for (libxsmm_xcopy_loop_j_ = N0; libxsmm_xcopy_loop_j_ < (N1); ++libxsmm_xcopy_loop_j_) { \
        XKERNEL(TYPE, TYPESIZE, OUT, IN, LDI, LDO, libxsmm_xcopy_loop_i_, libxsmm_xcopy_loop_j_, \
          libxsmm_xcopy_loop_src_, libxsmm_xcopy_loop_dst_); \
        *libxsmm_xcopy_loop_dst_ = *libxsmm_xcopy_loop_src_; \
      } \
    } \
  } \
  else { \
    unsigned int libxsmm_xcopy_loop_k_; \
    for (libxsmm_xcopy_loop_i_ = M0; libxsmm_xcopy_loop_i_ < (M1); ++libxsmm_xcopy_loop_i_) { \
      LIBXSMM_PRAGMA_NONTEMPORAL HINT_ALIGNED(OUT) \
      for (libxsmm_xcopy_loop_j_ = N0; libxsmm_xcopy_loop_j_ < (N1); ++libxsmm_xcopy_loop_j_) { \
        XKERNEL(TYPE, TYPESIZE, OUT, IN, LDI, LDO, libxsmm_xcopy_loop_i_, libxsmm_xcopy_loop_j_, \
          libxsmm_xcopy_loop_src_, libxsmm_xcopy_loop_dst_); \
        for (libxsmm_xcopy_loop_k_ = 0; libxsmm_xcopy_loop_k_ < (TYPESIZE); ++libxsmm_xcopy_loop_k_) { \
          libxsmm_xcopy_loop_dst_[libxsmm_xcopy_loop_k_] = libxsmm_xcopy_loop_src_[libxsmm_xcopy_loop_k_]; \
        } \
      } \
    } \
  } \
}

#define LIBXSMM_XCOPY_XALIGN(TYPE, TYPESIZE, XKERNEL, OUT, IN, LDI, LDO, M0, M1, N0, N1) { \
  if (0 == LIBXSMM_MOD2((LDO) * (TYPESIZE), LIBXSMM_ALIGNMENT) && 0 == LIBXSMM_MOD2((uintptr_t)(OUT), LIBXSMM_ALIGNMENT)) \
  { \
    LIBXSMM_XCOPY_LOOP(TYPE, TYPESIZE, XKERNEL, LIBXSMM_PRAGMA_VALIGNED_VARS, OUT, IN, LDI, LDO, M0, M1, N0, N1); \
  } \
  else { /* unaligned store */ \
    LIBXSMM_XCOPY_LOOP(TYPE, TYPESIZE, XKERNEL, LIBXSMM_XCOPY_LOOP_UNALIGNED, OUT, IN, LDI, LDO, M0, M1, N0, N1); \
  } \
}

#define LIBXSMM_XCOPY(XKERNEL, KERNEL_CALL, KERNEL, OUT, IN, TYPESIZE, LDI, LDO, TILE_M, TILE_N, M0, M1, N0, N1) { \
  if (0 != (KERNEL) && (TILE_M) == ((M1) - (M0)) && (TILE_N) == (N1) - (N0)) /* inner tile? */ { \
    const unsigned int libxsmm_xcopy_ldi_ = LDI, libxsmm_xcopy_ldo_ = LDO; \
    XKERNEL(char, TYPESIZE, OUT, IN, LDI, LDO, M0, N0, libxsmm_xcopy_src_, libxsmm_xcopy_dst_); \
    KERNEL_CALL(KERNEL, TYPESIZE, libxsmm_xcopy_src_, &libxsmm_xcopy_ldi_, libxsmm_xcopy_dst_, &libxsmm_xcopy_ldo_); \
  } \
  else { \
    switch(TYPESIZE) { \
      case 2: { \
        LIBXSMM_XCOPY_XALIGN(short, 2, XKERNEL, OUT, IN, LDI, LDO, M0, M1, N0, N1); \
      } break; \
      case 4: { \
        LIBXSMM_XCOPY_XALIGN(float, 4, XKERNEL, OUT, IN, LDI, LDO, M0, M1, N0, N1); \
      } break; \
      case 8: { \
        LIBXSMM_XCOPY_XALIGN(double, 8, XKERNEL, OUT, IN, LDI, LDO, M0, M1, N0, N1); \
      } break; \
      case 16: { \
        typedef struct dvec2_t { double value[2]; } dvec2_t; \
        LIBXSMM_XCOPY_XALIGN(dvec2_t, 16, XKERNEL, OUT, IN, LDI, LDO, M0, M1, N0, N1); \
      } break; \
      default: { \
        LIBXSMM_XCOPY_XALIGN(char, TYPESIZE, XKERNEL, OUT, IN, LDI, LDO, M0, M1, N0, N1); \
      } break; \
    } \
  } \
}

#define LIBXSMM_XCOPY_RECURSIVE(FN, XKERNEL_START, XKERNEL, KERNEL_CALL, KERNEL, \
  OUT, IN, TYPESIZE, LDI, LDO, TILE_M, TILE_N, M0, M1, N0, N1) \
{ \
  XKERNEL_START(untied) \
  if ((M1) <= ((M0) + (TILE_M)) && (N1) <= ((N0) + (TILE_N))) { \
    LIBXSMM_XCOPY(XKERNEL, KERNEL_CALL, KERNEL, OUT, IN, TYPESIZE, LDI, LDO, TILE_M, TILE_N, M0, M1, N0, N1); \
  } \
  else if (((M0) + (TILE_M)) < (M1)) { \
    const unsigned int libxsmm_xcopy_mi_ = (M0) + (TILE_M); \
    (FN)(KERNEL, OUT, IN, TYPESIZE, LDI, LDO, TILE_M, TILE_N, M0, libxsmm_xcopy_mi_, N0, N1); \
    (FN)(KERNEL, OUT, IN, TYPESIZE, LDI, LDO, TILE_M, TILE_N, libxsmm_xcopy_mi_, M1, N0, N1); \
  } \
  else { \
    const unsigned int libxsmm_xcopy_ni_ = (N0) + (TILE_N); \
    assert(((N0) + (TILE_N)) < (N1)); \
    (FN)(KERNEL, OUT, IN, TYPESIZE, LDI, LDO, TILE_M, TILE_N, M0, M1, N0, libxsmm_xcopy_ni_); \
    (FN)(KERNEL, OUT, IN, TYPESIZE, LDI, LDO, TILE_M, TILE_N, M0, M1, libxsmm_xcopy_ni_, N1); \
  } \
}


/** Initializes the transpose functionality; NOT thread-safe. */
LIBXSMM_API void libxsmm_trans_init(int archid);

/** Finalizes the transpose functionality; NOT thread-safe. */
LIBXSMM_API void libxsmm_trans_finalize(void);


/** Size of peeled chunks during transposing inner tiles. */
LIBXSMM_EXTERN_C LIBXSMM_RETARGETABLE unsigned int libxsmm_trans_tile[2/*DP/SP*/][2/*M,N*/][8/*size-range*/];

#endif /*LIBXSMM_TRANS_H*/

