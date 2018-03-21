add_executable(tester tests.cpp)
target_compile_features(tester PRIVATE cxx_std_17)
target_link_libraries(tester stdc++fs)

add_test(NAME single_dir 
         COMMAND tester 01_one_dir.test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)
