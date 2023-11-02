CPMAddPackage(
        NAME catch2
        GITHUB_REPOSITORY catchorg/Catch2
        VERSION 3.4.0
        GIT_SHALLOW YES
        GIT_PROGRESS YES
        EXCLUDE_FROM_ALL YES
        SYSTEM
        OPTIONS
            "CATCH_BUILD_STATIC_LIBRARY ON"
)

include(CTest)
enable_testing()

CPMAddPackage(
        NAME random123
        GITHUB_REPOSITORY DEShawResearch/random123
        GIT_TAG main
        GIT_SHALLOW YES
        GIT_PROGRESS YES
        EXCLUDE_FROM_ALL YES
        SYSTEM
        DOWNLOAD_ONLY YES
)

add_library(r123 INTERFACE IMPORTED)
target_include_directories(r123 INTERFACE ${random123_SOURCE_DIR}/include)

add_executable(testPhilox test_philox.cpp)
target_link_libraries(testPhilox PRIVATE philox Catch2::Catch2WithMain r123)
target_compile_options(testPhilox PRIVATE -Wall -pedantic -march=native)
add_test(NAME testPhilox COMMAND testPhilox)


include(CheckLanguage)
check_language(CUDA)

if (CMAKE_CUDA_COMPILER)
    message(STATUS "CUDA compiler found enabling GPU support")
    enable_language(CUDA)
    add_executable(testPhiloxGPU test_philox.cu
            test_philox.cu)
    target_compile_options(testPhiloxGPU PRIVATE --expt-relaxed-constexpr)
    target_link_libraries(testPhiloxGPU PRIVATE philox Catch2::Catch2WithMain)
    set_property(TARGET testPhiloxGPU PROPERTY CUDA_ARCHITECTURES native)
    set_property(TARGET testPhiloxGPU PROPERTY CUDA_STANDARD 20)
    set_property(TARGET testPhiloxGPU PROPERTY CXX_STANDARD 20)
    add_test(NAME testPhiloxGPU COMMAND testPhiloxGPU)
endif ()