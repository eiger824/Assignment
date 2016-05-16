cmake_minimum_required(VERSION 2.8)

project(Assignment)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${TMP}")

set(HEADERS
  src/statistics.hpp
  src/types.hpp
  )

set(SOURCES
  src/main.cpp
  src/statistics.cpp
  )
add_executable(TS_Analyzer
  ${HEADERS}
  ${SOURCES}
  )