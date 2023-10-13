/*
MIT License

Copyright (c) 2024 Marco Barbone

Permission is hereby granted, free of charge, to any person obtaining a copy
        of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#ifdef __CUDACC__
#define PHILOX_DEVICE      __device__
#define PHILOX_HOST        __host__
#define PHILOX_HOST_DEVICE PHILOX_HOST PHILOX_DEVICE
#else
#define PHILOX_DEVICE
#define PHILOX_HOST
#define PHILOX_HOST_DEVICE
#endif

#define PHILOX_INLINE inline __attribute__((always_inline))

#include "internal/philox2_64_10.h"
#include "internal/philox4_32_10.h"

#undef PHILOX_INLINE
#undef PHILOX_DEVICE
#undef PHILOX_HOST
#undef PHILOX_HOST_DEVICE
