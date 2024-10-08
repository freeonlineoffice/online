/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include <config.h>

#include <Simd.hpp>

#if ENABLE_SIMD
#  include <immintrin.h>
#endif

namespace simd {

bool HasAVX2 = false;

bool init()
{
#if ENABLE_SIMD
    __builtin_cpu_init();
    HasAVX2 = __builtin_cpu_supports ("avx2");
#endif
    return HasAVX2;
}

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
