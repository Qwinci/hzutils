#include <benchmark/benchmark.h>
#include <stddef.h>
#include <string.h>

#define CONCAT_HELPER(a, b) a ## b
#define CONCAT(a, b) CONCAT_HELPER(a, b)

#define HZ_MEM_FUNC(name) CONCAT(HZ_MEM_SSE2_PREFIX, name)

extern "C" void* HZ_MEM_FUNC(memset)(void* dest, int ch, size_t size) noexcept;
extern "C" void* HZ_MEM_FUNC(memcpy)(void* __restrict dest, const void* __restrict src, size_t size) noexcept;
extern "C" void* HZ_MEM_FUNC(memmove)(void* dest, const void* src, size_t size) noexcept;

template<auto Fn>
static void bench_memset(benchmark::State& state) {
	char* dst = new char[state.range(0)];

	for (auto _ : state) {
		Fn(dst, 'x', state.range(0));
	}

	benchmark::DoNotOptimize(dst);

	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));

	delete[] dst;
}

template<auto Fn>
static void bench_memcpy(benchmark::State& state) {
	char* src = new char[state.range(0)];
	char* dst = new char[state.range(0)];
	memset(src, 'x', state.range(0));

	for (auto _ : state) {
		Fn(dst, src, state.range(0));
	}

	benchmark::DoNotOptimize(src);
	benchmark::DoNotOptimize(dst);

	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));

	delete[] src;
	delete[] dst;
}

template<auto Fn>
static void bench_memmove(benchmark::State& state) {
	char* src = new char[state.range(0) + 1];
	char* dst = src + 1;
	memset(src, 'x', state.range(0));
	src[0] = 'a';
	src[1] = 'b';

	for (auto _ : state) {
		Fn(dst, src, state.range(0));
	}

	benchmark::DoNotOptimize(src);
	benchmark::DoNotOptimize(dst);

	assert(src[1] == 'a');
	assert(src[2] == 'b');

	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));

	delete[] src;
}

BENCHMARK(bench_memset<memset>)->Name("libc memset")->Range(8, 8<<10);
BENCHMARK(bench_memcpy<memcpy>)->Name("libc memcpy")->Range(8, 8<<10);
//BENCHMARK(bench_memcpy<memmove>)->Name("libc memmove")->Range(8, 8<<10);
//BENCHMARK(bench_memmove<memmove>)->Name("libc memmove overlap")->Range(8, 8<<10);

BENCHMARK(bench_memset<hz_sse2_memset>)->Name("hz sse2 memset")->Range(8, 8<<10);
BENCHMARK(bench_memcpy<hz_sse2_memcpy>)->Name("hz sse2 memcpy")->Range(8, 8<<10);
//BENCHMARK(bench_memcpy<hz_sse2_memmove>)->Name("hz sse2 memmove")->Range(8, 8<<10);
//BENCHMARK(bench_memmove<hz_sse2_memmove>)->Name("hz sse2 memmove overlap")->Range(8, 8<<10);

BENCHMARK_MAIN();
