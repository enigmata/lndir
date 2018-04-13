cmake_minimum_required(VERSION 3.8.2)

cmake_policy(SET CMP0048 NEW)

project(lndir
        VERSION 0.9.0
        LANGUAGES CXX)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(CheckCXXStdFilesystemExists)

enable_testing()

add_compile_options("-Wall" "-Wextra" "-Werror" "-O2")

check_cxx_std_filesystem_exists()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/version.hpp.in"
               "${CMAKE_CURRENT_BINARY_DIR}/src/version.hpp"
               @ONLY)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/src")

add_executable(lndir src/lndir.cpp)
target_compile_features(lndir PRIVATE cxx_std_17)
target_link_libraries(lndir stdc++fs)

install(TARGETS lndir
        RUNTIME DESTINATION bin)

add_subdirectory(test)
