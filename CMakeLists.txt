cmake_minimum_required(VERSION 3.8.2)

project(lndir LANGUAGES CXX)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(CheckCXXStdFilesystemExists)

add_compile_options("-Wall" "-Wextra" "-Werror" "-O2")

check_cxx_std_filesystem_exists()

add_executable(lndir src/lndir.cpp)
target_compile_features(lndir PRIVATE cxx_std_17)
target_link_libraries(lndir stdc++fs)

install(TARGETS lndir
        RUNTIME DESTINATION bin)

add_subdirectory(test)
