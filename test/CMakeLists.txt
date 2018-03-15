add_executable(tester tests.cpp)
target_compile_features(tester PRIVATE cxx_std_17)
target_link_libraries(tester stdc++fs)
