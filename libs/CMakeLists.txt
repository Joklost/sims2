if (NOT TARGET eigen)
    set(EIGEN_SPLIT_LARGE_TESTS OFF CACHE BOOL "Don't split Eigen tests")
    add_subdirectory(eigen)
endif ()

if (NOT TARGET geo)
    add_subdirectory(geo)
endif ()

if (NOT TARGET SFML)
    add_subdirectory(SFML/include/)
endif ()
# to build SFML, install these
# libx11-dev, freeglut3-d, libudev-dev, libopenal-dev, libflac-dev, libvorbis-dev, libvorbisenc2, oggvideotools, oggfwd