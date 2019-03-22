add_subdirectory(libs)

add_executable(test_sims2 main.cpp test.cpp)

target_link_libraries(test_sims2 PUBLIC sims2 Catch2)
target_include_directories(test_sims2 PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestSims2 COMMAND test_sims2)