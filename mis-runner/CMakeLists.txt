cmake_minimum_required(VERSION 3.16)
project(bench_ogdf CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OGDF_ROOT "$ENV{HOME}/OGDF")
set(OGDF_BUILD "${OGDF_ROOT}/build")

add_executable(bench src/bench.cpp)

target_include_directories(bench PRIVATE
  "${OGDF_BUILD}/include/ogdf-release"
  "${OGDF_ROOT}/include"
)

target_link_directories(bench PRIVATE "${OGDF_BUILD}")
target_link_libraries(bench PRIVATE OGDF COIN)

set_target_properties(bench PROPERTIES
  BUILD_RPATH "${OGDF_BUILD}"
)
