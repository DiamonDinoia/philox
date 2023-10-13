CPMAddPackage(
        NAME catch2
        GITHUB_REPOSITORY catchorg/Catch2
        VERSION 2.13.7
        GIT_SHALLOW YES
        GIT_PROGRESS YES
        EXCLUDE_FROM_ALL YES
        SYSTEM
)
include(CTest)
enable_testing()

set(TEST_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

file(MAKE_DIRECTORY ${TEST_INCLUDE_DIR})

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
target_link_libraries(testPhilox PRIVATE philox Catch2::Catch2 r123)
target_include_directories(testPhilox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(testPhilox PRIVATE -Wall -pedantic -march=native)
add_test(NAME testPhilox COMMAND testPhilox)
