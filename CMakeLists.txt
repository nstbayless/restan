cmake_minimum_required(VERSION 2.0)
project (restan)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(.)

file(GLOB _SRC
    "*.cpp"
    "hmcmc/*.cpp"
    "pi/*.cpp"
)

add_executable(restan
  ${_SRC}
)

target_link_libraries(restan openblas adept)