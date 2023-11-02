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

#include <catch2/catch_all.hpp>

static inline constexpr auto TESTS = 1 << 15;

template <typename T>
__global__ void testSeeding(std::uint64_t seed) {
    T rng(seed, 0, 0);
}

template <typename T>
__global__ void testGeneration(std::uint64_t seed, std::uint64_t* out) {
    T rng(seed, 0, 0);
    for (auto i = 0; i < TESTS; ++i) { out[i] = rng(); }
}

TEST_CASE("Philox4 32 INIT") {
    const auto seed = std::random_device()();
    INFO("SEED: " << seed);

    testSeeding<Philox4x32_10><<<1, 1>>>(seed);
    REQUIRE(cudaDeviceSynchronize() == cudaSuccess);
}

TEST_CASE("Philox2 64 INIT") {
    const auto seed = std::random_device()();
    INFO("SEED: " << seed);

    testSeeding<Philox2x64_10><<<1, 1>>>(seed);
    REQUIRE(cudaDeviceSynchronize() == cudaSuccess);
}

TEST_CASE("Philox4 32 INIT  GENERATE") {
    const auto seed = std::random_device()();
    INFO("SEED: " << seed);
    Philox4x32_10  reference(seed, 0, 0);
    std::uint64_t* out;

    REQUIRE(cudaMallocManaged(&out, TESTS * sizeof(std::uint64_t)) == cudaSuccess);
    REQUIRE(cudaDeviceSynchronize() == cudaSuccess);

    testGeneration<Philox4x32_10><<<1, 1>>>(seed, out);
    REQUIRE(cudaDeviceSynchronize() == cudaSuccess);

    for (auto i = 0; i < TESTS; ++i) { REQUIRE(out[i] == reference()); }
    REQUIRE(cudaFree(out) == cudaSuccess);
}

TEST_CASE("Philox2 64 INIT GENERATE") {
    const auto seed = std::random_device()();
    INFO("SEED: " << seed);
    Philox2x64_10  reference(seed, 0, 0);
    std::uint64_t* out;

    REQUIRE(cudaMallocManaged(&out, TESTS * sizeof(std::uint64_t)) == cudaSuccess);
    REQUIRE(cudaDeviceSynchronize() == cudaSuccess);

    testGeneration<Philox2x64_10><<<1, 1>>>(seed, out);
    REQUIRE(cudaDeviceSynchronize() == cudaSuccess);

    for (auto i = 0; i < TESTS; ++i) { REQUIRE(out[i] == reference()); }

    REQUIRE(cudaFree(out) == cudaSuccess);
}
