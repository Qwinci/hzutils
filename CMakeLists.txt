cmake_minimum_required(VERSION 4.0)
project(hzutils2)

set(CMAKE_CXX_STANDARD 23)

add_library(hzutils2 INTERFACE)
target_include_directories(hzutils2 INTERFACE include)

if(HZUTILS_ENABLE_TESTING)
	add_executable(hzutils2_test main.cpp)
	target_compile_options(hzutils2_test PRIVATE -Wall -Wextra -ffreestanding)
	target_link_libraries(hzutils2_test PUBLIC hzutils2)

	add_executable(bench bench_main.cpp src/mem_sse2.cpp src/mem_scalar.cpp)
	target_compile_options(bench PRIVATE -Wall -Wextra)
	target_compile_definitions(bench PRIVATE HZ_MEM_SSE2_PREFIX=hz_sse2_ HZ_MEM_SCALAR_PREFIX=hz_scalar_)
	target_link_libraries(bench PRIVATE benchmark)
endif()
