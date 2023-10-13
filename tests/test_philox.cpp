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

#include <philox/philox.h>
// do not reorder these include
#include <Random123/philox.h>

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

static constexpr auto tests = 1 << 15;

class Philox4x32_10_test : public Philox4x32_10 {
   public:
    Philox4x32_10_test(std::uint64_t seed) : Philox4x32_10(seed, 0, 0) {}

    Philox4x32_10_test() : Philox4x32_10(0, 0, 0) {}

    uint4&      internal_counter_ref() { return m_counter; }
    uint2&      internal_key_ref() { return m_key; }
    static auto ten_rounds(uint4 ctr, uint2 key) { return Philox4x32_10::ten_rounds(ctr, key); }
};

class Philox2x64_10_test : public Philox2x64_10 {
   public:
    Philox2x64_10_test(std::uint64_t seed) : Philox2x64_10(seed, 0, 0) {}

    Philox2x64_10_test() : Philox2x64_10(0, 0, 0) {}

    auto&       internal_counter_ref() { return m_counter; }
    auto&       internal_key_ref() { return m_key; }
    static auto ten_rounds(uint2 ctr, std::uint64_t key) { return Philox2x64_10::ten_rounds(ctr, key); }
};

TEST_CASE("Performance") {
    const auto seed = std::random_device()();
    INFO("SEED: " << seed);

    Philox4x32_10_test rng{};
    rng.seed(seed, 0, 0);
    auto result = 0ULL;
    BENCHMARK("Philox4x32_10") {
        for (int i = 0; i < tests; ++i) { result += rng(); }
    };

    {
        r123::Philox4x32           reference;
        r123::Philox4x32::ctr_type counter = {rng.internal_counter_ref().x, rng.internal_counter_ref().y,
                                              rng.internal_counter_ref().z, rng.internal_counter_ref().w};
        r123::Philox4x32::key_type key     = {rng.internal_key_ref().x, rng.internal_key_ref().y};

        BENCHMARK("Philox4x32_10 ref") {
            for (int i = 0; i < tests; ++i) { counter = reference(counter, key); }
        };
        REQUIRE(counter[0] != 0);
    }
    {
        r123::Philox2x64           reference;
        r123::Philox2x64::ctr_type counter = {rng.internal_counter_ref().x, rng.internal_counter_ref().y};
        r123::Philox2x64::key_type key     = {rng.internal_key_ref().x};

        BENCHMARK("Philox2x64_10 ref") {
            for (int i = 0; i < (tests >> 1); ++i) { counter = reference(counter, key); }
        };
        REQUIRE(counter[0] != 0);
    }
    {
        Philox2x64_10_test rng{};
        rng.seed(seed, 0, 0);
        BENCHMARK("Philox2x64_10") {
            for (int i = 0; i < tests >> 1; ++i) { result += rng(); }
        };
    }
    REQUIRE(result != 0);
}

TEST_CASE("Seeding") {
    Philox4x32_10_test engine;
    auto&              state = engine.internal_counter_ref();

    REQUIRE(state.x == 0U);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 0U);

    engine.discard(1 * 4ULL);
    REQUIRE(state.x == 1U);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 0U);

    engine.seed(3331, 0, 5 * 4ULL);
    REQUIRE(state.x == 5U);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 0U);
}

TEST_CASE("Discard") {
    Philox4x32_10_test engine;
    auto&              state = engine.internal_counter_ref();

    REQUIRE(state.x == 0U);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 0U);

    engine.discard(UINT_MAX * 4ULL);
    REQUIRE(state.x == UINT_MAX);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 0U);

    engine.discard(UINT_MAX * 4ULL);
    REQUIRE(state.x == UINT_MAX - 1);
    REQUIRE(state.y == 1U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 0U);

    engine.discard(2 * 4ULL);
    REQUIRE(state.x == 0U);
    REQUIRE(state.y == 2U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 0U);

    state.x = UINT_MAX;
    state.y = UINT_MAX;
    state.z = UINT_MAX;
    engine.discard(1 * 4ULL);
    REQUIRE(state.x == 0U);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 1U);

    state.x = UINT_MAX;
    state.y = UINT_MAX;
    state.z = UINT_MAX;
    engine.discard(1 * 4ULL);
    REQUIRE(state.x == 0U);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 2U);

    state.x = 123;
    state.y = 456;
    state.z = 789;
    state.w = 999;
    engine.discard(1 * 4ULL);
    REQUIRE(state.x == 124U);
    REQUIRE(state.y == 456U);
    REQUIRE(state.z == 789U);
    REQUIRE(state.w == 999U);

    state.x = 123;
    state.y = 0;
    state.z = 0;
    state.w = 0;
    engine.discard(1 * 4ULL);
    REQUIRE(state.x == 124U);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 0U);

    state.x = UINT_MAX - 1;
    state.y = 2;
    state.z = 3;
    state.w = 4;
    engine.discard(((1ull << 32) + 2ull) * 4ULL);
    REQUIRE(state.x == 0U);
    REQUIRE(state.y == 4U);
    REQUIRE(state.z == 3U);
    REQUIRE(state.w == 4U);
}

TEST_CASE("Discard sequence") {
    Philox4x32_10_test engine;
    auto&              state = engine.internal_counter_ref();

    engine.discard_subsequence(UINT_MAX);
    REQUIRE(state.x == 0U);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == UINT_MAX);
    REQUIRE(state.w == 0U);

    engine.discard_subsequence(UINT_MAX);
    REQUIRE(state.x == 0U);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == UINT_MAX - 1);
    REQUIRE(state.w == 1U);

    engine.discard_subsequence(2);
    REQUIRE(state.x == 0U);
    REQUIRE(state.y == 0U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 2U);

    state.x = 123;
    state.y = 456;
    state.z = 789;
    state.w = 999;
    engine.discard_subsequence(1);
    REQUIRE(state.x == 123U);
    REQUIRE(state.y == 456U);
    REQUIRE(state.z == 790U);
    REQUIRE(state.w == 999U);

    state.x = 1;
    state.y = 2;
    state.z = UINT_MAX - 1;
    state.w = 4;
    engine.discard_subsequence((1ull << 32) + 2ull);
    REQUIRE(state.x == 1U);
    REQUIRE(state.y == 2U);
    REQUIRE(state.z == 0U);
    REQUIRE(state.w == 6U);
}

TEST_CASE("Correctness 4x32") {
    const std::uint64_t        seed = std::random_device()();
    Philox4x32_10_test         engine{seed};
    r123::Philox4x32           reference;
    r123::Philox4x32::ctr_type counter = {engine.internal_counter_ref().x, engine.internal_counter_ref().y,
                                          engine.internal_counter_ref().z, engine.internal_counter_ref().w};
    r123::Philox4x32::key_type key     = {engine.internal_key_ref().x, engine.internal_key_ref().y};

    auto reference_rounds = reference(counter, key);
    auto result_rounds    = engine.ten_rounds(engine.internal_counter_ref(), engine.internal_key_ref());
    REQUIRE(reference_rounds[0] == result_rounds.x);
    REQUIRE(reference_rounds[1] == result_rounds.y);
    REQUIRE(reference_rounds[2] == result_rounds.z);
    REQUIRE(reference_rounds[3] == result_rounds.w);
}

TEST_CASE("Correctness 2x64") {
    const std::uint64_t        seed = std::random_device()();
    Philox2x64_10_test         engine{seed};
    r123::Philox2x64           reference;
    r123::Philox2x64::ctr_type counter = {engine.internal_counter_ref().x, engine.internal_counter_ref().y};
    r123::Philox2x64::key_type key     = {engine.internal_key_ref()};

    auto reference_rounds = reference(counter, key);
    auto result_rounds    = engine.ten_rounds(engine.internal_counter_ref(), engine.internal_key_ref());
    REQUIRE(reference_rounds[0] == result_rounds.x);
    REQUIRE(reference_rounds[1] == result_rounds.y);
}
