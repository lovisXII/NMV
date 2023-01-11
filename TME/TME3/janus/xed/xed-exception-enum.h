/*BEGIN_LEGAL 
Copyright 2004-2016 Intel Corporation. Use of this code is subject to
the terms and conditions of the What If Pre-Release License Agreement,
which is available here:
https://software.intel.com/en-us/articles/what-if-pre-release-license-agreement
or refer to the LICENSE.txt file.
END_LEGAL */
/// @file xed-exception-enum.h

// This file was automatically generated.
// Do not edit this file.

#if !defined(_XED_EXCEPTION_ENUM_H_)
# define _XED_EXCEPTION_ENUM_H_
#include "xed-common-hdrs.h"
typedef enum {
  XED_EXCEPTION_INVALID,
  XED_EXCEPTION_AVX512_E1,
  XED_EXCEPTION_AVX512_E10,
  XED_EXCEPTION_AVX512_E10NF,
  XED_EXCEPTION_AVX512_E11,
  XED_EXCEPTION_AVX512_E11NF,
  XED_EXCEPTION_AVX512_E12,
  XED_EXCEPTION_AVX512_E12NP,
  XED_EXCEPTION_AVX512_E1NF,
  XED_EXCEPTION_AVX512_E2,
  XED_EXCEPTION_AVX512_E3,
  XED_EXCEPTION_AVX512_E3NF,
  XED_EXCEPTION_AVX512_E4,
  XED_EXCEPTION_AVX512_E4NF,
  XED_EXCEPTION_AVX512_E5,
  XED_EXCEPTION_AVX512_E5NF,
  XED_EXCEPTION_AVX512_E6,
  XED_EXCEPTION_AVX512_E6NF,
  XED_EXCEPTION_AVX512_E7NM,
  XED_EXCEPTION_AVX512_E7NM128,
  XED_EXCEPTION_AVX512_E9NF,
  XED_EXCEPTION_AVX512_K20,
  XED_EXCEPTION_AVX512_K21,
  XED_EXCEPTION_AVX_TYPE_1,
  XED_EXCEPTION_AVX_TYPE_11,
  XED_EXCEPTION_AVX_TYPE_12,
  XED_EXCEPTION_AVX_TYPE_2,
  XED_EXCEPTION_AVX_TYPE_2D,
  XED_EXCEPTION_AVX_TYPE_3,
  XED_EXCEPTION_AVX_TYPE_4,
  XED_EXCEPTION_AVX_TYPE_4M,
  XED_EXCEPTION_AVX_TYPE_5,
  XED_EXCEPTION_AVX_TYPE_5L,
  XED_EXCEPTION_AVX_TYPE_6,
  XED_EXCEPTION_AVX_TYPE_7,
  XED_EXCEPTION_AVX_TYPE_8,
  XED_EXCEPTION_MMX_FP,
  XED_EXCEPTION_MMX_FP_16ALIGN,
  XED_EXCEPTION_MMX_MEM,
  XED_EXCEPTION_MMX_NOFP,
  XED_EXCEPTION_MMX_NOFP2,
  XED_EXCEPTION_MMX_NOMEM,
  XED_EXCEPTION_SSE_TYPE_1,
  XED_EXCEPTION_SSE_TYPE_2,
  XED_EXCEPTION_SSE_TYPE_2D,
  XED_EXCEPTION_SSE_TYPE_3,
  XED_EXCEPTION_SSE_TYPE_4,
  XED_EXCEPTION_SSE_TYPE_4M,
  XED_EXCEPTION_SSE_TYPE_5,
  XED_EXCEPTION_SSE_TYPE_7,
  XED_EXCEPTION_LAST
} xed_exception_enum_t;

/// This converts strings to #xed_exception_enum_t types.
/// @param s A C-string.
/// @return #xed_exception_enum_t
/// @ingroup ENUM
XED_DLL_EXPORT xed_exception_enum_t str2xed_exception_enum_t(const char* s);
/// This converts strings to #xed_exception_enum_t types.
/// @param p An enumeration element of type xed_exception_enum_t.
/// @return string
/// @ingroup ENUM
XED_DLL_EXPORT const char* xed_exception_enum_t2str(const xed_exception_enum_t p);

/// Returns the last element of the enumeration
/// @return xed_exception_enum_t The last element of the enumeration.
/// @ingroup ENUM
XED_DLL_EXPORT xed_exception_enum_t xed_exception_enum_t_last(void);
#endif
