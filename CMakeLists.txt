cmake_minimum_required (VERSION 2.6)

set (PROJECT meshbuilder_3d)

project (${PROJECT})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")


# Add Boost
set(BOOST_LIBRARYDIR /s/ls2/home/users/gector/libs/boost_old/lib/)
set(BOOST_ROOT /s/ls2/home/users/gector/libs/boost_1_42_0m)
set(BOOST_INCLUDEDIR /s/ls2/home/users/gector/libs/boost_1_42_0m)
include_directories(${BOOST_INCLUDEDIR})
#find_package(Boost COMPONENTS system filesystem REQUIRED)

# Add Triangle
SET(TRIANGLE_SRC_DIR "${CMAKE_SOURCE_DIR}/libs/triangle")

include_directories(${TRIANGLE_SRC_DIR})
link_directories (${TRIANGLE_SRC_DIR})

# Add Tetgen
SET(TETGEN_SRC_DIR "${CMAKE_SOURCE_DIR}/libs/tetgen1.5.0")

include_directories(${TETGEN_SRC_DIR})
link_directories (${TETGEN_SRC_DIR})


set(MY_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(MY_FIGURES_DIR ${CMAKE_SOURCE_DIR}/src/figures)

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
        ${MY_FIGURES_DIR}/cross_fracture.h
        ${MY_FIGURES_DIR}/cube.h
)

set (SOURCES
	${MY_SOURCE_DIR}/mesh.cpp)

add_executable (${PROJECT} ${HEADERS} ${SOURCES} ${TRIANGLE_SRC_DIR}/triangle.o)
target_link_libraries (${PROJECT} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${TETGEN_SRC_DIR}/libtet.a)

