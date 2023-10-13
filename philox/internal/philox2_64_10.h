/*
MIT License

Copyright (c) 2024 Marco Barbone

Permission is hereby granted, free of charge, to any person obtaining a copy
        of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
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
// Copyright (c) 2017-2023 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

/*
Copyright 2010-2011, D. E. Shaw Research.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions, and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions, and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

* Neither the name of D. E. Shaw Research nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <bit>
#include <iostream>
#include <limits>

#include "common.h"

class Philox2x64_10 : protected philox {
   public:
    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr explicit Philox2x64_10(
        philox::uint64_t seed_value = PHILOX4x32_DEFAULT_SEED, philox::uint64_t subsequence = 0,
        philox::uint64_t offset = 0) {
        seed(seed_value, subsequence, offset);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void seed(philox::uint64_t       seed_value,
                                                         const philox::uint64_t subsequence,
                                                         const philox::uint64_t offset) {
        m_key = seed_value;
        restart(subsequence, offset);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void restart(const philox::uint64_t subsequence,
                                                            const philox::uint64_t offset) {
        m_counter  = {0, 0};
        m_result   = {0, 0};
        m_substate = 0;
        discard_subsequence_impl(subsequence);
        discard_impl(offset);
        m_result = ten_rounds(m_counter, m_key);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard(philox::uint64_t offset) {
        discard_impl(offset);
        m_result = ten_rounds(m_counter, m_key);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard_subsequence(philox::uint64_t subsequence) {
        discard_subsequence_impl(subsequence);
        m_result = ten_rounds(m_counter, m_key);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr philox::uint64_t next() {
        const auto ret = m_result.data[m_substate++];
        if (m_substate == 2) {
            discard_state();
            m_result = ten_rounds(m_counter, m_key);
        }
        return ret;
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr philox::uint64_t operator()() { return next(); }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr philox::uint64_t max() {
        return std::numeric_limits<philox::uint64_t>::max();
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr philox::uint64_t min() {
        return std::numeric_limits<philox::uint64_t>::min();
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr double uniform() {
        return static_cast<double>(operator()() >> 11) * 0x1.0p-53;
    }

   protected:
    static constexpr philox::uint64_t PHILOX4x32_DEFAULT_SEED{0xdeadbeefdeadbeefULL};
    static constexpr philox::uint64_t PHILOX_M2x64_0{0xD2B74407B1CE6E93};
    static constexpr philox::uint64_t PHILOX_W64_0{0x9E3779B97F4A7C15};
    static constexpr philox::uint64_t PHILOX_W64_1{0xBB67AE8584CAA73B};

    union alignas(16) uint2 {
        struct {
            philox::uint64_t x;
            philox::uint64_t y;
        };
        philox::uint64_t  data[2];
        philox::uint128_t m128i;
    };

    uint2            m_counter{};
    uint2            m_result{};
    philox::uint64_t m_key{};
    philox::uint8_t  m_substate{};

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard_subsequence_impl(philox::uint64_t subsequence) {
        const auto temp = m_counter.y;
        m_counter.y += subsequence;
        m_counter.x += (m_counter.y < temp);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard_state() { bump_counter(); }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard_state(philox::uint64_t offset) {
        const auto temp = m_counter.x;
        m_counter.x += offset;
        m_counter.y += (m_counter.x < temp);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard_impl(philox::uint64_t offset) {
        // Adjust offset for subset
        m_substate += offset & 1;
        philox::uint64_t counter_offset = offset >> 1;
        counter_offset += m_substate < 2 ? 0 : 1;
        m_substate += m_substate < 2 ? 0 : -2;
        // Discard states
        discard_state(counter_offset);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void bump_counter() { m_counter.m128i++; }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr uint2 toUint2(uint128_t x) { return std::bit_cast<uint2>(x); }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr uint2 mulhilo64(philox::uint64_t x, philox::uint64_t y) {
        auto xy = static_cast<unsigned __int128>(x) * y;
        return toUint2(xy);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr philox::uint64_t bumpkey(philox::uint64_t key) {
        key += PHILOX_W64_0;
        return key;
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr uint2 single_round(const uint2           &counter,
                                                                         const philox::uint64_t key) {
        const auto result = mulhilo64(PHILOX_M2x64_0, counter.x);
        return {result.y ^ counter.y ^ key, result.x};
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr uint2 ten_rounds(uint2 counter, philox::uint64_t key) {
        counter = single_round(counter, key);
        key     = bumpkey(key);  // 1
        counter = single_round(counter, key);
        key     = bumpkey(key);  // 2
        counter = single_round(counter, key);
        key     = bumpkey(key);  // 3
        counter = single_round(counter, key);
        key     = bumpkey(key);  // 4
        counter = single_round(counter, key);
        key     = bumpkey(key);  // 5
        counter = single_round(counter, key);
        key     = bumpkey(key);  // 6
        counter = single_round(counter, key);
        key     = bumpkey(key);  // 7
        counter = single_round(counter, key);
        key     = bumpkey(key);  // 8
        counter = single_round(counter, key);
        key     = bumpkey(key);             // 9
        return single_round(counter, key);  // 10
    }
};
