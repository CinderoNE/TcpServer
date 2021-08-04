cmake_minimum_required(VERSION 3.16)
PROJECT(TCP_SERVER CXX)


IF(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Wno-sign-compare -Wno-unused-but-set-variable -Wno-array-bounds")
MESSAGE(STATUS "BUILD_TYPE = " ${CMAKE_BUILD_TYPE})
MESSAGE(STATUS "The cmake_cxx_flags is: ${CMAKE_CXX_FLAGS}")

ADD_SUBDIRECTORY(src)