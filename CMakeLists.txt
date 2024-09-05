cmake_minimum_required(VERSION 3.28)
project(hzutils)

set(CMAKE_CXX_STANDARD 20)

option(ENABLE_TESTING "Enable testing" OFF)

add_library(hzutils INTERFACE)
target_include_directories(hzutils INTERFACE include)

if(ENABLE_TESTING)
	include(FetchContent)
	FetchContent_Declare(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest
		GIT_TAG 3ef16ef8b30f52a8f1ff677faa1730e76835aa40
	)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)

	enable_testing()

	add_executable(hzutils_test main.cpp)
	target_compile_options(hzutils_test PRIVATE -fsanitize=undefined,address)
	target_link_libraries(hzutils_test PRIVATE GTest::gtest_main hzutils)
	target_link_options(hzutils_test PRIVATE -fsanitize=undefined,address)

	include(GoogleTest)
	gtest_discover_tests(hzutils_test)
endif()
