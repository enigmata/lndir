cmake_minimum_required(VERSION 3.8.2)

include(CheckCXXSourceCompiles)

project(lndir LANGUAGES CXX)

add_compile_options("-Wall" "-Wextra" "-Werror")

try_compile(SUPPORT_STD_FILESYTEM ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src/build_check_std_filesystem.cpp LINK_LIBRARIES stdc++fs CXX_STANDARD 17)
if (SUPPORT_STD_FILESYTEM)
    message(STATUS "Compiler supports std::filesystem")
elseif (NOT SUPPORT_STD_FILESYTEM)
    message(FATAL_ERROR "Compiler does not support std::filesystem")
endif(SUPPORT_STD_FILESYTEM)

add_executable(lndir src/lndir.cpp)
target_compile_features(lndir PRIVATE cxx_std_17)
target_link_libraries(lndir stdc++fs)
