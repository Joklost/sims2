if (NOT TARGET eigen)
    set(EIGEN_SPLIT_LARGE_TESTS OFF CACHE BOOL "Don't split Eigen tests")
    add_subdirectory(eigen)
endif()

if (NOT TARGET geo)
    add_subdirectory(geo)
endif()
