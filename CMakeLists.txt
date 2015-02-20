cmake_minimum_required(VERSION 2.6)
project(meshbuilder)

# Set build directory
set(BINDIR ${CMAKE_CURRENT_SOURCE_DIR}/../bin)

MAKE_DIRECTORY(${BINDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINDIR})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")


# Add Triangle
SET(TRIANGLE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/triangle/")

add_library (triangle ${TRIANGLE_SRC_DIR}/triangle.c) 
include_directories(${TRIANGLE_SRC_DIR})
link_directories (${TRIANGLE_SRC_DIR})

# Add Fade
#SET(FADE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/fadeRelease/")
#include_directories(${FADE_SRC_DIR}/include_fade2d)
#link_directories (${FADE_SRC_DIR}/linux_x86_64)


# Add Tetgen
SET(TETGEN_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/tetgen1.5.0/")

include_directories(${TETGEN_SRC_DIR})
link_directories (${TETGEN_SRC_DIR})

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/tetgen1.5.0/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

# Sources and headers
set(MY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MY_FIGURES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/figures)

set (HEADERS
    ${MY_SOURCE_DIR}/point.h
    ${MY_SOURCE_DIR}/edge.h
    ${MY_SOURCE_DIR}/facet.h
    ${MY_SOURCE_DIR}/figure.h
    ${MY_SOURCE_DIR}/mesh.h
    ${MY_SOURCE_DIR}/meshsplitter.h
    ${MY_FIGURES_DIR}/fracture.h
    ${MY_FIGURES_DIR}/fracture_cross_array.h
    ${MY_FIGURES_DIR}/rect_boundary.h
    ${MY_FIGURES_DIR}/layered_boundary.h
    ${MY_FIGURES_DIR}/cross_fracture.h
    ${MY_FIGURES_DIR}/cube.h
    ${MY_FIGURES_DIR}/ply_model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../profile/Profile.h
)

set (SOURCES
        ${MY_SOURCE_DIR}/mesh.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../profile/src/Profile.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/tetgen1.5.0/)

add_executable (meshbuilder ${HEADERS} ${SOURCES})
target_link_libraries (meshbuilder tet triangle)

