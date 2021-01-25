# Created by the script cgal_create_CMakeLists
# This is the CMake script for compiling a set of CGAL applications.

project( cgal-classification-benchmark )

cmake_minimum_required(VERSION 3.1)

# CGAL and its components
find_package( CGAL REQUIRED )

if ( NOT CGAL_FOUND )
  message(STATUS "This project requires the CGAL library, and will not be compiled.")
  return()  
endif()
include( ${CGAL_USE_FILE} )

find_package( Boost REQUIRED )
if ( NOT Boost_FOUND )
  message(STATUS "This project requires the Boost library, and will not be compiled.")
  return()  
endif()

find_package(Eigen3 3.1.0)
include(CGAL_Eigen_support)
if (NOT TARGET CGAL::Eigen_support)
  message(STATUS "NOTICE: This project requires the Eigen library, and will not be compiled.")
  return()
endif()

find_package(TBB QUIET)
include(CGAL_TBB_support)
if(NOT TARGET CGAL::TBB_support)
  message(STATUS "NOTICE: This project requires the TBB library, and will not be compiled.")
  return()
endif()

include_directories( BEFORE include )

add_executable(benchmark "benchmark.cpp")
target_link_libraries(benchmark CGAL::CGAL CGAL::Eigen_support CGAL::TBB_support)

