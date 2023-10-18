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
#include <cstdint>
#include <iostream>

#include "common.h"

class Philox4x32_10 : protected philox {
   public:
    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr explicit Philox4x32_10(
        philox::uint64_t seed_value = PHILOX4x32_DEFAULT_SEED, philox::uint64_t subsequence = 0,
        philox::uint64_t offset = 0) noexcept {
        seed(seed_value, subsequence, offset);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr Philox4x32_10(const Philox4x32_10 &other) noexcept            = default;
    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr Philox4x32_10(Philox4x32_10 &&other) noexcept                 = default;
    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr Philox4x32_10 &operator=(const Philox4x32_10 &other) noexcept = default;
    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr Philox4x32_10 &operator=(Philox4x32_10 &&other) noexcept      = default;
    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr ~Philox4x32_10() noexcept                                     = default;

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void seed(philox::uint64_t       seed_value,
                                                         const philox::uint64_t subsequence,
                                                         const philox::uint64_t offset) {
        m_key = toUint2(seed_value);
        restart(subsequence, offset);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void restart(const philox::uint64_t subsequence,
                                                            const philox::uint64_t offset) noexcept {
        m_counter  = {0, 0, 0, 0};
        m_result   = {0, 0, 0, 0};
        m_substate = 0;
        discard_subsequence_impl(subsequence);
        discard_impl(offset);
        m_result = ten_rounds(m_counter, m_key);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard(philox::uint64_t offset) noexcept {
        discard_impl(offset);
        m_result = ten_rounds(m_counter, m_key);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard_subsequence(philox::uint64_t subsequence) noexcept {
        discard_subsequence_impl(subsequence);
        m_result = ten_rounds(m_counter, m_key);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr philox::uint32_t next_32() noexcept {
        const auto ret = m_result.data[m_substate++];
        if (m_substate == 4) {
            discard_state();
            m_result = ten_rounds(m_counter, m_key);
        }
        return ret;
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr philox::uint64_t next_64() noexcept {
        return std::bit_cast<philox::uint64_t>(uint2{next_32(), next_32()});
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr philox::uint64_t operator()() noexcept { return next_64(); }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr philox::uint64_t max() noexcept {
        return std::numeric_limits<philox::uint64_t>::max();
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr philox::uint64_t min() noexcept {
        return std::numeric_limits<philox::uint64_t>::min();
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr double uniform() noexcept {
        return static_cast<double>(operator()() >> 11) * 0x1.0p-53;
    }

   protected:
    static constexpr philox::uint64_t PHILOX4x32_DEFAULT_SEED{0xdeadbeefdeadbeefULL};
    static constexpr philox::uint32_t PHILOX_M4x32_0{0xD2511F53U};
    static constexpr philox::uint32_t PHILOX_M4x32_1{0xCD9E8D57U};
    static constexpr philox::uint32_t PHILOX_W32_0{0x9E3779B9U};
    static constexpr philox::uint32_t PHILOX_W32_1{0xBB67AE85U};

    union alignas(16) uint4 {
        struct {
            philox::uint32_t x;
            philox::uint32_t y;
            philox::uint32_t z;
            philox::uint32_t w;
        };
        philox::uint32_t  data[4];
        philox::uint128_t m128i;
    };

    struct alignas(8) uint2 {
        philox::uint32_t x;
        philox::uint32_t y;
    };

    uint4           m_counter{};
    uint4           m_result{};
    uint2           m_key{};
    philox::uint8_t m_substate{};

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard_subsequence_impl(philox::uint64_t subsequence) noexcept {
        const auto [lo, hi] = toUint2(subsequence);
        const auto temp     = m_counter.z;
        m_counter.z += lo;
        m_counter.w += hi + (m_counter.z < temp);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard_state() noexcept { bump_counter(); }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard_state(philox::uint64_t offset) noexcept {
        const auto [lo, hi] = toUint2(offset);
        const auto temp     = m_counter;
        m_counter.x += lo;
        m_counter.y += hi + (m_counter.x < temp.x);
        m_counter.z += (m_counter.y < temp.y);
        m_counter.w += (m_counter.z < temp.z);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void discard_impl(philox::uint64_t offset) noexcept {
        // Adjust offset for subset
        m_substate += offset & 3;
        philox::uint64_t counter_offset = offset >> 2;
        counter_offset += m_substate < 4 ? 0 : 1;
        m_substate += m_substate < 4 ? 0 : -4;
        // Discard states
        discard_state(counter_offset);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE constexpr void bump_counter() noexcept { m_counter.m128i++; }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr uint2 toUint2(philox::uint64_t x) noexcept {
        return std::bit_cast<uint2>(x);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr uint2 mulhilo32(philox::uint32_t x, philox::uint32_t y) noexcept {
        auto xy = static_cast<philox::uint64_t>(x) * y;
        return toUint2(xy);
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr uint2 bumpkey(uint2 key) noexcept {
        key.x += PHILOX_W32_0;
        key.y += PHILOX_W32_1;
        return key;
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr uint4

    single_round(const uint4 &counter, const uint2 &key) noexcept {
        // Source: Random123
        const auto [lo0, hi0] = mulhilo32(PHILOX_M4x32_0, counter.x);
        const auto [lo1, hi1] = mulhilo32(PHILOX_M4x32_1, counter.z);
        return {hi1 ^ counter.y ^ key.x, lo1, hi0 ^ counter.w ^ key.y, lo0};
    }

    PHILOX_INLINE PHILOX_HOST_DEVICE static constexpr uint4 ten_rounds(uint4 counter, uint2 key) noexcept {
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
