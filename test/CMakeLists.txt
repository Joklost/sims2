add_subdirectory(libs)

add_executable(test_sims2 main.cpp test.cpp)

target_link_libraries(test_sims2 PUBLIC sims2 Catch2)
target_include_directories(test_sims2 PUBLIC ${PROJECT_SOURCE_DIR}/test)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_maps/map1.bmp ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_maps/map1_modified.bmp ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_maps/map2.bmp ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gen_map ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_test(NAME TestSims2 COMMAND test_sims2)